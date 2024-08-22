/*
 * Copyright 2021 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "LPC55S36.h"
#ifdef ENABLE_IEC60730B
#include "project_setup_lpcxpresso55s36.h"
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void common_startup(void);
extern int32_t main(void);
extern void WatchdogEnable(void);
extern void WatchdogDisable(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Start
 *
 * @param void
 *
 * @return None
 *
 * This function sets up watchdog, calls all of the needed starup routines and then
 * branches to the main process.
 */
void start(void)
{

#ifdef ENABLE_IEC60730B
    /* Update of watchdog configuration */
#if WATCHDOG_ENABLED
    WatchdogEnable();
#else
    WatchdogDisable();
#endif
#endif
    /* Copy any vector or data sections that need to be in RAM */
    common_startup();

    /* Jump to main process */
    main();

    /* No actions to perform after this so wait forever */
    while (1)
        ;
}
