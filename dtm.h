#ifndef __DTM_H__
#define __DTM_H__

extern volatile bool is_msb_read;
extern volatile uint16_t dtm_cmd_from_uart;
extern bool is_dtm_mode_inited;

int dtm_main(void);
void dtm_timer_start(void);
void dtm_timer_stop(void);

#endif

