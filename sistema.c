#include <stdio.h>
#include <stdlib.h>

#include "sistema.h"
#include "menu.h"
#include "entrada.h"
#include "configuracion.h"

struct Sistema
{
    int activo;
};

static void ejecutarMenuPrincipal(Sistema *sistema);

static void ejecutarMenuOperativo(void);

static void ejecutarMenuGeneral(void);


Sistema *crearSistema(void)
{
    Sistema *nuevoSistema = malloc(sizeof(Sistema));

    if (nuevoSistema == NULL)
    {
        printf("Error: no fue posible reservar memoria para el sistema.\n");

        return NULL;
    }

    nuevoSistema->activo = SISTEMA_ACTIVO;

    return nuevoSistema;
}


void ejecutarSistema(Sistema *sistema)
{
    if (sistema == NULL)
    {
        return;
    }

    ejecutarMenuPrincipal(sistema);
}


static void ejecutarMenuPrincipal(Sistema *sistema)
{
    int opcion;

    while (sistema->activo == SISTEMA_ACTIVO)
    {
        mostrarMenuPrincipal();

        opcion = leerEntero(
            "Seleccione una opcion: "
        );

        switch (opcion)
        {
            case OPCION_MENU_OPERATIVAS:

                ejecutarMenuOperativo();

                break;

            case OPCION_MENU_GENERALES:

                ejecutarMenuGeneral();

                break;

            case OPCION_MENU_SALIR:

                sistema->activo = SISTEMA_INACTIVO;

                printf("\n");
                printf("Saliendo del sistema...\n");

                break;

            default:

                printf("\n");
                printf("Opcion invalida.\n");

                break;
        }
    }
}


static void ejecutarMenuOperativo(void)
{
    int opcion = 0;

    while (opcion != OPCION_VOLVER_OPERATIVAS)
    {
        mostrarMenuOperativo();

        opcion = leerEntero(
            "Seleccione una opcion: "
        );

        switch (opcion)
        {
            case 1:

                mostrarOpcionNoImplementada(
                    "Gestion de catalogo"
                );

                break;

            case 2:

                mostrarOpcionNoImplementada(
                    "Gestion de usuarios"
                );

                break;

            case 3:

                mostrarOpcionNoImplementada(
                    "Historial de prestamos"
                );

                break;

            case 4:

                mostrarOpcionNoImplementada(
                    "Vencimiento de prestamos"
                );

                break;

            case 5:

                mostrarOpcionNoImplementada(
                    "Estadisticas"
                );

                break;

            case OPCION_VOLVER_OPERATIVAS:

                printf("\nVolviendo al menu principal...\n");

                break;

            default:

                printf("\nOpcion invalida.\n");

                break;
        }
    }
}


static void ejecutarMenuGeneral(void)
{
    int opcion = 0;

    while (opcion != OPCION_VOLVER_GENERALES)
    {
        mostrarMenuGeneral();

        opcion = leerEntero(
            "Seleccione una opcion: "
        );

        switch (opcion)
        {
            case 1:

                mostrarOpcionNoImplementada(
                    "Busqueda simple"
                );

                break;

            case 2:

                mostrarOpcionNoImplementada(
                    "Busqueda avanzada"
                );

                break;

            case 3:

                mostrarOpcionNoImplementada(
                    "Prestamo de ejemplares"
                );

                break;

            case 4:

                mostrarOpcionNoImplementada(
                    "Devolucion de ejemplar"
                );

                break;

            case OPCION_VOLVER_GENERALES:

                printf("\nVolviendo al menu principal...\n");

                break;

            default:

                printf("\nOpcion invalida.\n");

                break;
        }
    }
}


void destruirSistema(Sistema *sistema)
{
    if (sistema != NULL)
    {
        free(sistema);
    }
}