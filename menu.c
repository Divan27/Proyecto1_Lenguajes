#include <stdio.h>

#include "menu.h"
#include "configuracion.h"

void mostrarMenuPrincipal(void)
{
    printf("\n");
    printf("=========================================\n");
    printf("       %s\n", NOMBRE_SISTEMA);
    printf("=========================================\n");
    printf("1. Opciones Operativas\n");
    printf("2. Opciones Generales\n");
    printf("3. Salir\n");
    printf("=========================================\n");
}

void mostrarMenuOperativo(void)
{
    printf("\n");
    printf("=========================================\n");
    printf("          OPCIONES OPERATIVAS\n");
    printf("=========================================\n");
    printf("1. Gestion de catalogo\n");
    printf("2. Gestion de usuarios\n");
    printf("3. Historial de prestamos\n");
    printf("4. Vencimiento de prestamos\n");
    printf("5. Estadisticas\n");
    printf("6. Volver\n");
    printf("=========================================\n");
}

void mostrarMenuGeneral(void)
{
    printf("\n");
    printf("=========================================\n");
    printf("           OPCIONES GENERALES\n");
    printf("=========================================\n");
    printf("1. Busqueda simple\n");
    printf("2. Busqueda avanzada\n");
    printf("3. Prestamo de ejemplares\n");
    printf("4. Devolucion de ejemplar\n");
    printf("5. Volver\n");
    printf("=========================================\n");
}

void mostrarOpcionNoImplementada(const char *nombreOpcion)
{
    printf("\n-----------------------------------------\n");
    printf("%s\n", nombreOpcion);
    printf("Esta funcion sera implementada proximamente.\n");
    printf("-----------------------------------------\n");
}