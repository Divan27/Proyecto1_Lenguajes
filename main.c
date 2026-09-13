#include <stddef.h>
#include "sistema.h"

int main(void)
{
    Sistema *sistema = crearSistema();

    if (sistema == NULL)
    {
        return 1;
    }

    ejecutarSistema(sistema);

    destruirSistema(sistema);

    return 0;
}