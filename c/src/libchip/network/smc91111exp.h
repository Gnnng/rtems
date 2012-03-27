/*
 *  $Id$
 */

#ifndef _SMC91111_EXP_H_
#define _SMC91111_EXP_H_

typedef struct scmv91111_configuration {
  void                     *baseaddr;
  int                       irq;
  unsigned int              pio;
  unsigned int              ctl_rspeed;
  unsigned int              ctl_rfduplx;
  unsigned int              ctl_autoneg;
} scmv91111_configuration_t;

#endif  /* _SMC_91111_EXP_H_ */


