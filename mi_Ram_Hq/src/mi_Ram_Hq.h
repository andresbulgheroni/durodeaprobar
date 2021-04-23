/*
 * mi_Ram_Hq.h
 *
 *  Created on: 21 abr. 2021
 *      Author: utnso
 */

#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

t_config* leer_config(void);
t_log* crear_log(char* dir);

#endif /* MI_RAM_HQ_H_ */
