#ifndef SISTEMA_H
#define SISTEMA_H

typedef struct Sistema Sistema;

Sistema *crearSistema(void);

void ejecutarSistema(Sistema *sistema);

void destruirSistema(Sistema *sistema);

#endif