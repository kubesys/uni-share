#ifndef __CONTROL_H
#define __CONTROL_H

void *source_control(void *arg);
int delta_for_mlu(int up_limit, int user_current, int share);
int delta_for_gpu(int up_limit, int user_current, int share);
static void change_token(int delta);
void rate_limiter(int grids, int blocks);


#endif