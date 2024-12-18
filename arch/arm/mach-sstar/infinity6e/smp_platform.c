/*
* smp_platform.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
/*
 * cedric_smp.c
 *
 *  Created on: 2015�~4��30��
 *      Author: Administrator
 */


#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>

#include <asm/cacheflush.h>
#include <asm/cp15.h>
#include <asm/smp_plat.h>

#include "ms_platform.h"

extern int infinity6e_platform_cpu_kill(unsigned int cpu);
extern void infinity6e_platform_cpu_die(unsigned int cpu);
extern int infinity6e_platform_cpu_disable(unsigned int cpu);

extern void infinity6e_secondary_startup(void);
void infinity6e_smp_init_cpus(void);
void infinity6e_smp_prepare_cpus(unsigned int max_cpus);
//extern volatile int __cpuinitdata pen_release;
extern volatile int pen_release;
extern void Chip_Flush_CacheAll(void);

#ifdef CONFIG_PM_SLEEP
extern int suspend_status;
#endif
//#define SCU_PHYS 0x16000000 /*Cedric*/
#define SCU_PHYS 0x16000000 /*MACAN*/ // SCU PA = 0x16004000

#if 0
#define ARM_MPIDR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 0, r0, c0, c0, 5\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })
#endif

static inline void cpu_enter_lowpower(void)
{
	unsigned int v;

	printk(KERN_INFO "ms_hotplug.c  cpu_enter_lowpower: in\n");

	flush_cache_all();
	asm volatile(
	"	mcr	p15, 0, %1, c7, c5, 0\n"
	"	mcr	p15, 0, %1, c7, c10, 4\n"
	/*
	 * Turn off coherency
	 */
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	bic	%0, %0, #0x20\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	bic	%0, %0, %2\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	  : "=&r" (v)
	  : "r" (0), "Ir" (CR_C)
	  : "cc");

	printk(KERN_INFO "ms_hotplug.c  cpu_enter_lowpower: out\n");
}

static inline void cpu_leave_lowpower(void)
{
	unsigned int v;

	printk(KERN_INFO "ms_hotplug.c  cpu_leave_lowpower: in\n");

	asm volatile(	"mrc	p15, 0, %0, c1, c0, 0\n"
	"	orr	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	orr	%0, %0, #0x20\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	  : "=&r" (v)
	  : "Ir" (CR_C)
	  : "cc");

	printk(KERN_INFO "ms_hotplug.c  cpu_leave_lowpower: out\n");
}

static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
       //printk("ms_hotplug.c  platform_do_lowpower: before %d cpu go into WFI, spurious =%d  \n", cpu, *spurious);

	/*
	 * there is no power-control hardware on this platform, so all
	 * we can do is put the core into WFI; this is safe as the calling
	 * code will have already disabled interrupts
	 */
	for (;;) {
		/*
		 * here's the WFI
		 */
		asm("wfi\n"
		    :
		    :
		    : "memory", "cc");
		//printk("ms_hotplug.c  platform_do_lowpower: %d cpu  wake up, spurious =%d  \n", cpu, *spurious);

		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}

		//printk("ms_hotplug.c  platform_do_lowpower: %d cpu  wake up error(cpuID != pen_release), spurious =%d  \n", cpu, *spurious);

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;
	}
}

int infinity6e_platform_cpu_kill(unsigned int cpu)
{
	return 1;
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void __ref infinity6e_platform_cpu_die(unsigned int cpu)
{
	int spurious = 0;
//	printk(KERN_ERR "[%s]\n",__FUNCTION__);

	//printk("ms_hotplug.c  platform_cpu_die: in cpu = %d \n", cpu);

	Chip_Flush_CacheAll();

	/*
	 * we're ready for shutdown now, so do it
	 */
	//cpu_enter_lowpower();
	platform_do_lowpower(cpu, &spurious);

	/*
	 * bring this CPU back into the world of cache
	 * coherency, and then restore interrupts
	 */
	//cpu_leave_lowpower();

	printk(KERN_INFO "ms_hotplug.c  platform_cpu_die: out cpu = %d \n", cpu);

	if (spurious)
		pr_warn("CPU%u: %u spurious wakeup calls\n", cpu, spurious);
}

int infinity6e_platform_cpu_disable(unsigned int cpu)
{
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */

	printk(KERN_INFO "ms_hotplug.c  platform_cpu_disable: in cpu = %d \n", cpu);
	return cpu == 0 ? -EPERM : 0;
}


/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
//volatile int __cpuinitdata pen_release = -1;

static void __iomem *scu_base_addr(void)
{
//	printk(KERN_ERR "[%s]\n",__FUNCTION__);
	return (void __iomem *)(IO_ADDRESS(SCU_PHYS));
}

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
//static void __cpuinit write_pen_release(int val)
static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static DEFINE_SPINLOCK(boot_lock);

void infinity6e_secondary_init(unsigned int cpu)
{
//	printk(KERN_ERR "[%s]\n",__FUNCTION__);

	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	//gic_secondary_init(0);

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

#define SECOND_START_ADDR_HI			0x1F20404C
#define SECOND_START_ADDR_LO			0x1F204050
#define SECOND_MAGIC_NUMBER_ADDR 	    0x1F204058

int infinity6e_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;
	printk(KERN_INFO "[%s]\n",__FUNCTION__);
	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * This is really belt and braces; we hold unintended secondary
	 * CPUs in the holding pen until we're ready for them.  However,
	 * since we haven't sent them a soft interrupt, they shouldn't
	 * be there.
	 */
	write_pen_release(cpu_logical_map(cpu));

	do{
		OUTREG16(SECOND_MAGIC_NUMBER_ADDR, 0xBABE);
	}while(INREG16(SECOND_MAGIC_NUMBER_ADDR)!=0xBABE);

#ifdef CONFIG_PM_SLEEP
	if (suspend_status) {
		infinity6e_smp_init_cpus();
		infinity6e_smp_prepare_cpus(2);
	}
#endif
	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */
	printk(KERN_DEBUG "[%s] ipi\n",__FUNCTION__);
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

#ifdef CONFIG_PM_SLEEP
	if (suspend_status) {
		suspend_status = 0;
	}
#endif

	return pen_release != -1 ? -ENOSYS : 0;
}


#include <asm/smp_scu.h>
#include <linux/io.h>


#define SCU_CTRL		0x00

void infinity6e_smp_prepare_cpus(unsigned int max_cpus)
{
	int i;
//	u32 scu_ctrl;
//	printk(KERN_ERR "[%s]\n",__FUNCTION__);
	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
	{
		set_cpu_present(i, true);
	}

#if defined(CONFIG_CEDRIC_MASTER0_ONLY_PATCH)
    __raw_writel(0xe0000000, scu_base_addr() + 0x40);
    __raw_writel(0xe0100000, scu_base_addr() + 0x44);
    scu_ctrl = __raw_readl(scu_base_addr() + SCU_CTRL);
    scu_ctrl |= 0x02;
    __raw_writel(scu_ctrl, scu_base_addr() + SCU_CTRL);
    printk(KERN_WARNING"SCU: Filter to Master0 only\n");
#endif

	scu_enable(scu_base_addr()); // SCU PA = 0x16000000

//	printk("[306]!!  scu_enable\n");

	/*
	 * Write the address of secondary startup into the
	 * system-wide flags register. The boot monitor waits
	 * until it receives a soft interrupt, and then the
	 * secondary CPU branches to this address.
	 */
	do{
		OUTREG16(SECOND_START_ADDR_LO,(virt_to_phys(infinity6e_secondary_startup) & 0xFFFF));
	}while(INREG16(SECOND_START_ADDR_LO)!=(virt_to_phys(infinity6e_secondary_startup) & 0xFFFF));

	do{
		OUTREG16(SECOND_START_ADDR_HI,(virt_to_phys(infinity6e_secondary_startup)>>16));
	}while(INREG16(SECOND_START_ADDR_HI)!=(virt_to_phys(infinity6e_secondary_startup)>>16));

	__cpuc_flush_kern_all();
}

void infinity6e_smp_init_cpus(void)
{

	void __iomem *scu_base =scu_base_addr();
	unsigned int i, ncores;
	printk(KERN_INFO "[%s]\n",__FUNCTION__);
	ncores = scu_base ? scu_get_core_count(scu_base) : 1;

	for (i = 0; i < 2; i++)
	{
		set_cpu_possible(i, true);
	}
}

#ifdef CONFIG_LH_RTOS
#define GICD_IGROUPR        0x080

#define GICC_CTLR           0x00
#define GICC_PMR            0x04
#define GICC_BPR            0x08

#define GICD_BASE           0x16001000
#define GICC_BASE           0x16002000

#define GICD_WRITEL(a,v)    (*(volatile unsigned int *)(u32)(GICD_BASE + a) = (v))
#define GICC_WRITEL(a,v)    (*(volatile unsigned int *)(u32)(GICC_BASE + a) = (v))

void infinity6e_secondary_gic(void)
{
	/* Diable GICC */
    GICC_WRITEL(GICC_CTLR,0x00000000);

    /* set interrupt priority mask level = 15, if the interrupt priority is larger than this value, it will be sent to CPU*/
    GICC_WRITEL(GICC_PMR , 0xf0);

    /* enable group priority */
    GICC_WRITEL(GICC_BPR , 3);

    /* LH: Set all interrupts are Grp 1 for IRQ */
    GICD_WRITEL(GICD_IGROUPR , ~0x0);

    /* LH: run in Secure Mode to Enable Grp1, Ackctl and FIQEn */
    GICC_WRITEL(GICC_CTLR,0x000001EA); //non-shared
}
#endif

struct smp_operations __initdata infinity6e_smp_ops  = {
    .smp_init_cpus      = infinity6e_smp_init_cpus,
    .smp_prepare_cpus   = infinity6e_smp_prepare_cpus,
    .smp_secondary_init = infinity6e_secondary_init,
    .smp_boot_secondary = infinity6e_boot_secondary,

#ifdef CONFIG_HOTPLUG_CPU
    .cpu_kill           = infinity6e_platform_cpu_kill,
    .cpu_die            = infinity6e_platform_cpu_die,
    .cpu_disable        = infinity6e_platform_cpu_disable,
#endif
};
