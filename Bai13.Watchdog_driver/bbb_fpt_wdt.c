#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/moduleparam.h>
#include <linux/pm_runtime.h>
#include <linux/watchdog.h>

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout, "FPT_WDT: Watchdog cannot be stopped once started (default="
		__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static unsigned int timer_margin;
module_param(timer_margin, uint, 0);
MODULE_PARM_DESC(timer_margin, "FPT_WDT: Initial watchdog timeout (in seconds)");

/*Prescaler for counter*/
#define BBB_WDT_PTV 						0
/*Timer margin min, max and default in second*/
#define BBB_WDT_TIMER_MARGIN_MAX			(24*60*60) /*One day*/
#define BBB_WDT_TIMER_MARGIN_MIN			(1U)
#define BBB_WDT_TIMER_MARGIN_DEFAULT		(10U)

/*The default clock for WDT in Hz*/
#define BBB_WDT_DEF_CLK						(32768U)
/*Convert the value of Load Value register in tick and counter register in second*/
#define BBB_WDT_GET_WLDR_VAL(secs)			(0xFFFFFFFF - ((secs) * (BBB_WDT_DEF_CLK/(1<<BBB_WDT_PTV))) + 1)
#define BBB_WDT_GET_WCCR_SECS(val)			((0xFFFFFFFF - (val) + 1) / (BBB_WDT_DEF_CLK/(1<<BBB_WDT_PTV)))

/*Define the WDT's register offset*/
#define BBB_WDT_WIDR		(0x00U)
#define BBB_WDT_WDSC		(0x10U)
#define BBB_WDT_WDST		(0x14U)
#define BBB_WDT_WISR		(0x18U)
#define BBB_WDT_WIER		(0x1CU)
#define BBB_WDT_WCLR		(0x24U)
#define BBB_WDT_WCRR		(0x28U)
#define BBB_WDT_WLDR		(0x2CU)
#define BBB_WDT_WTGR		(0x30U)
#define BBB_WDT_WWPS		(0x34U)
#define BBB_WDT_WDLY		(0x44U)
#define BBB_WDT_WSPR		(0x48U)
/*Define register mask in WWPS register*/
#define BBB_WDT_WWPS_WCLR_MASK		(1<<0)
#define BBB_WDT_WWPS_WCRR_MASK		(1<<1)
#define BBB_WDT_WWPS_WLDR_MASK		(1<<2)
#define BBB_WDT_WWPS_WTGR_MASK		(1<<3)
#define BBB_WDT_WWPS_WSPR_MASK		(1<<4)
#define BBB_WDT_WWPS_WDLY_MASK		(1<<5)

/*Define bit position in WCLR register*/
#define BBB_WDT_WCLR_PRE_POS		(5U)
#define BBB_WDT_WCLR_PTV_POS		(2U)

/*Struct for WDT driver*/
struct bbb_Wdt_dev
{
	struct watchdog_device 		bbb_WdogDev;
	void __iomem    			*bbb_Base;          /* physical */
	struct device   			*bbb_Dev;
	bool						bbb_WdtUsers;
	int							bbb_WdtTriggerValue;
	struct mutex				bbb_Lock;		/* to avoid races with PM */
};

/*=================START OF WATCHDOG CONTROL FUNCTION===================*/
/*Check the guide line to write WDT API in /Document/Watchdog*/

/*!
* @brief:		Function for enabling watchdog
* @param[in]:	wdtDev A pointer to the bbb_Wdt_dev configuration structure
* @param[out]:	None
* @return:		None
*/
static void bbb_WdtEnable(struct bbb_Wdt_dev *wdtDev)
{
	void __iomem    *base = wdtDev->bbb_Base;
	/* Sequence to enable the watchdog (Check on AM33XX RM)*/
	writel_relaxed(0xBBBB, base + BBB_WDT_WSPR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WSPR_MASK)
		cpu_relax();
	writel_relaxed(0x4444, base + BBB_WDT_WSPR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WSPR_MASK)
		cpu_relax();

	pr_emerg("FPT_WDT: Watchdog enable sub-function");
}

/*!
* @brief:		Function for disabling watchdog
* @param[in]:	wdtDev A pointer to the bbb_Wdt_dev configuration structure
* @param[out]:	None
* @return:		None
*/

static void bbb_WdtDisable(struct bbb_Wdt_dev *wdtDev)
{
	void __iomem    *base = wdtDev->bbb_Base;
	/* Sequence to disable the watchdog (Check on AM33XX RM)*/
	writel_relaxed(0xAAAA, base + BBB_WDT_WSPR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WSPR_MASK)
		cpu_relax();
	writel_relaxed(0x5555, base + BBB_WDT_WSPR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WSPR_MASK)
		cpu_relax();

	pr_emerg("FPT_WDT: Watchdog disable sub-function");
}

/*!
* @brief:		Function for setting timer watchdog
* @param[in]:	wdtDev A pointer to the bbb_Wdt_dev configuration structure
* @param[in]:	timeout: Watchdog timeout in second
* @param[out]:	None
* @return:		None
*/

static void bbb_WdtSetTimer(struct bbb_Wdt_dev *wdtDev, unsigned int timeout)
{
	void __iomem    *base = wdtDev->bbb_Base;
	unsigned int load_value = BBB_WDT_GET_WLDR_VAL(timeout);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WLDR_MASK)
		cpu_relax();	
	writel_relaxed(load_value, base + BBB_WDT_WLDR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WLDR_MASK)
		cpu_relax();	

	pr_emerg("FPT_WDT: Watchdog set timer sub-function");
}

/*!
* @brief:		Function for reload timer watchdog
* @param[in]:	wdtDev A pointer to the bbb_Wdt_dev configuration structure
* @param[out]:	None
* @return:		None
*/

static void bbb_WdtReload(struct bbb_Wdt_dev *wdtDev)
{
	void __iomem    *base = wdtDev->bbb_Base;
	wdtDev->bbb_WdtTriggerValue = ~wdtDev->bbb_WdtTriggerValue;
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WTGR_MASK)
		cpu_relax();	
	writel_relaxed(wdtDev->bbb_WdtTriggerValue, base + BBB_WDT_WTGR);
	/*Wait for command complete*/
	while(readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WTGR_MASK)
		cpu_relax();	

	pr_emerg("FPT_WDT: Watchdog reload sub-function");
}

/*!
* @brief:		Function for Start watchdog
* @param[in]:	wdtDev A pointer to the watchdog_device configuration structure
* @param[out]:	None
* @return:		0
*/

static int bbb_WdtStart(struct watchdog_device *wdog)
{
	/*The watchdog_get_drvdata function allows you to retrieve driver specific data.
	The argument of this function is the watchdog device where you want to retrieve
	data from. The function returns the pointer to the driver specific data.*/
	struct bbb_Wdt_dev *wdtDev = watchdog_get_drvdata(wdog);
	void __iomem *base = wdtDev->bbb_Base;

	mutex_lock(&wdtDev->bbb_Lock);

	wdtDev->bbb_WdtUsers = true;

	pm_runtime_get_sync(wdtDev->bbb_Dev);

	/*Disable watchdog first*/
	bbb_WdtDisable(wdtDev);
	/* initialize prescaler */
	while (readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WCLR_MASK)
		cpu_relax();	
	writel_relaxed((1 << BBB_WDT_WCLR_PRE_POS) | (BBB_WDT_PTV << BBB_WDT_WCLR_PTV_POS), base + BBB_WDT_WCLR);
	/*Wait for command complete*/
	while (readl_relaxed(base + BBB_WDT_WWPS) & BBB_WDT_WWPS_WCLR_MASK)
		cpu_relax();		
	/*Set timeout*/
	bbb_WdtSetTimer(wdtDev, wdog->timeout);
	/* Trigger loading of new timeout value */
	bbb_WdtReload(wdtDev);
	/*Enable watchdog*/
	bbb_WdtEnable(wdtDev);
	pr_emerg("FPT_WDT: Watchdog Start main-function");

	mutex_unlock(&wdtDev->bbb_Lock);
	return 0;
}

/*!
* @brief:		Function for Stop watchdog
* @param[in]:	wdtDev A pointer to the watchdog_device configuration structure
* @param[out]:	None
* @return:		0
*/

static int bbb_WdtStop(struct watchdog_device *wdog)
{
	struct bbb_Wdt_dev *wdtDev = watchdog_get_drvdata(wdog);
	mutex_lock(&wdtDev->bbb_Lock);
	
	/*Disable watchdog*/
	bbb_WdtDisable(wdtDev);	
	pm_runtime_put_sync(wdtDev->bbb_Dev);	
	wdtDev->bbb_WdtUsers = false;
	pr_emerg("FPT_WDT: Watchdog Stop main-function");

	mutex_unlock(&wdtDev->bbb_Lock);
	return 0;	
}

/*!
* @brief:		Function for writing value (ping) watchdog
* @param[in]:	wdtDev A pointer to the watchdog_device configuration structure
* @param[out]:	None
* @return:		0
*/

static int bbb_WdtPing(struct watchdog_device *wdog)
{
	struct bbb_Wdt_dev *wdtDev = watchdog_get_drvdata(wdog);
	mutex_lock(&wdtDev->bbb_Lock);

	bbb_WdtReload(wdtDev);
	pr_emerg("FPT_WDT: Watchdog Ping main-function");

	mutex_unlock(&wdtDev->bbb_Lock);

	return 0;		
}

/*!
* @brief:		Function for setting timeout for watchdog
* @param[in]:	wdtDev A pointer to the watchdog_device configuration structure
* @param[in]:	timeout: Watchdog timeout in second
* @param[out]:	None
* @return:		0
*/

static int bbb_WdtSetTimeout(struct watchdog_device *wdog, unsigned int timeout)
{
	struct bbb_Wdt_dev *wdtDev = watchdog_get_drvdata(wdog);
	mutex_lock(&wdtDev->bbb_Lock);

	bbb_WdtDisable(wdtDev);
	bbb_WdtSetTimer(wdtDev, timeout);
	bbb_WdtEnable(wdtDev);
	bbb_WdtReload(wdtDev);
	wdog->timeout = timeout;
	pr_emerg("FPT_WDT: Watchdog Setting timeout main-function");

	mutex_unlock(&wdtDev->bbb_Lock);

	return 0;		
}

/*!
* @brief:		Function for getting time left in second of counter watchdog
* @param[in]:	wdtDev A pointer to the watchdog_device configuration structure
* @param[out]:	None
* @return:		time left
*/

// static unsigned int bbb_WdtGetTimeleft(struct watchdog_device *wdog)
// {
// 	struct bbb_Wdt_dev *wdtDev = watchdog_get_drvdata(wdog);
// 	mutex_lock(&wdtDev->bbb_Lock);
// 	void __iomem *base = wdtDev->bbb_Base;
// 	u32 bbb_WdtCntValue;

// 	bbb_WdtCntValue = readl_relaxed(base + BBB_WDT_WCRR);
// 	pr_emerg("FPT_WDT: Watchdog Getting timeleft main-function");
// 	mutex_unlock(&wdtDev->bbb_Lock);
// 	return BBB_WDT_GET_WCCR_SECS(bbb_WdtCntValue);
// }

/*=================END OF WATCHDOG CONTROL FUNCTION===================*/
/*Driver attachment procedure*/

/*Struct watchdog info*/
static const struct watchdog_info bbb_WdtInfo = {
	.options = WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE | WDIOF_KEEPALIVEPING,
	.identity = "BBB Watchdog",
};

/*Function assignment structure*/
static const struct watchdog_ops bbb_WdtOps = {
	.owner		= THIS_MODULE,
	.start		= bbb_WdtStart,
	.stop		= bbb_WdtStop,
	.ping		= bbb_WdtPing,
	.set_timeout	= bbb_WdtSetTimeout,
	// .get_timeleft	= bbb_WdtGetTimeleft
};

/*Probe (driver initialize) function*/
static int bbb_WdtProbe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;
	struct bbb_Wdt_dev *wdt_Dev;

	wdt_Dev = devm_kzalloc(&pdev->dev, sizeof(*wdt_Dev), GFP_KERNEL);
	if (!wdt_Dev)
		return -ENOMEM;	

	wdt_Dev->bbb_WdtUsers	= false;
	wdt_Dev->bbb_Dev		= &pdev->dev;
	wdt_Dev->bbb_WdtTriggerValue	= 0x1234;
	mutex_init(&wdt_Dev->bbb_Lock);

	/* reserve static register mappings */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	wdt_Dev->bbb_Base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(wdt_Dev->bbb_Base))
		return PTR_ERR(wdt_Dev->bbb_Base);

	wdt_Dev->bbb_WdogDev.info 		=	&bbb_WdtInfo;
	wdt_Dev->bbb_WdogDev.ops 		= 	&bbb_WdtOps;
	wdt_Dev->bbb_WdogDev.min_timeout 	= 	BBB_WDT_TIMER_MARGIN_MIN;
	wdt_Dev->bbb_WdogDev.max_timeout 	= 	BBB_WDT_TIMER_MARGIN_MAX;
	wdt_Dev->bbb_WdogDev.parent 		= 	&pdev->dev;	
	/*Check if the passing timeout is valid*/
	if (watchdog_init_timeout(&wdt_Dev->bbb_WdogDev, timer_margin, &pdev->dev) < 0)
		wdt_Dev->bbb_WdogDev.timeout = BBB_WDT_TIMER_MARGIN_DEFAULT;	
	/*To set the WDOG_NO_WAY_OUT status bit (before registering your watchdog timer device)*/
	watchdog_set_nowayout(&wdt_Dev->bbb_WdogDev, nowayout);

	pm_runtime_enable(wdt_Dev->bbb_Dev);
	pm_runtime_get_sync(wdt_Dev->bbb_Dev);	

	/*The watchdog_set_drvdata function allows you to add driver specific data. The
	arguments of this function are the watchdog device where you want to add the
	driver specific data to and a pointer to the data itself.
	Add to the data field in watchdog_device struct*/
	watchdog_set_drvdata(&wdt_Dev->bbb_WdogDev, wdt_Dev);
	/*Add to the data field of platform_device struct*/
	platform_set_drvdata(pdev, wdt_Dev);
	bbb_WdtDisable(wdt_Dev);

	/*Register the watchdog device*/
	ret = watchdog_register_device(&wdt_Dev->bbb_WdogDev);
	if (ret) {
		pm_runtime_disable(wdt_Dev->bbb_Dev);
		return ret;
	}	
	pr_info("BBB Watchdog Timer Rev 0x%02x: initial timeout %d sec\n",
		readl_relaxed(wdt_Dev->bbb_Base + BBB_WDT_WIDR) & 0xFF,
		wdt_Dev->bbb_WdogDev.timeout);	

	pm_runtime_put(wdt_Dev->bbb_Dev);
	return 0;
}

/*Remove driver function*/

static int bbb_WdtRemove(struct platform_device *pdev)
{
	struct bbb_Wdt_dev *wdev = platform_get_drvdata(pdev);

	bbb_WdtStop(&wdev->bbb_WdogDev);
	pm_runtime_disable(wdev->bbb_Dev);
	watchdog_unregister_device(&wdev->bbb_WdogDev);
	devm_kfree(&pdev->dev, wdev);

	return 0;
}

static const struct of_device_id bbb_wdt_of_match[] = {
	{ .compatible = "ti,omap3-wdt", },
	{},
};

MODULE_DEVICE_TABLE(of, bbb_wdt_of_match);

static struct platform_driver bbb_wdt_driver = {
	.probe          = bbb_WdtProbe,
	.remove         = bbb_WdtRemove,
	.driver         = {
		.name   = "bbb_FPT_S_wdt",
		.of_match_table = bbb_wdt_of_match,
	},
};

module_platform_driver(bbb_wdt_driver);

MODULE_AUTHOR("SinhTV3");
MODULE_LICENSE("GPL");