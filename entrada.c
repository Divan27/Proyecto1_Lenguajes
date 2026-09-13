#include <stdio.h>
#include <stdlib.h>

#include "entrada.h"
#include "configuracion.h"

char *leerLineaDinamica(void)
{
    char *texto = NULL;

    int capacidad = TAMANIO_BLOQUE_LECTURA;
    int cantidadCaracteres = 0;
    int caracter;

    texto = malloc(capacidad * sizeof(char));

    if (texto == NULL)
    {
        return NULL;
    }

    while ((caracter = getchar()) != '\n' && caracter != EOF)
    {
        if (cantidadCaracteres + 1 >= capacidad)
        {
            int nuevaCapacidad = capacidad + TAMANIO_BLOQUE_LECTURA;

            char *nuevoTexto = realloc(
                texto,
                nuevaCapacidad * sizeof(char)
            );

            if (nuevoTexto == NULL)
            {
                free(texto);

                return NULL;
            }

            texto = nuevoTexto;
            capacidad = nuevaCapacidad;
        }

        texto[cantidadCaracteres] = (char) caracter;

        cantidadCaracteres++;
    }

    texto[cantidadCaracteres] = '\0';

    return texto;
}

int leerEntero(const char *mensaje)
{
    char *texto = NULL;

    char *finalConversion = NULL;

    long numero;

    int numeroValido = 0;

    while (!numeroValido)
    {
        printf("%s", mensaje);

        texto = leerLineaDinamica();

        if (texto == NULL)
        {
            printf("Error al reservar memoria.\n");

            return -1;
        }

        numero = strtol(
            texto,
            &finalConversion,
            10
        );

        if (
            finalConversion != texto &&
            *finalConversion == '\0'
        )
        {
            numeroValido = 1;
        }
        else
        {
            printf("\nEntrada invalida. Debe ingresar un numero.\n\n");
        }

        free(texto);
        texto = NULL;
    }

    return (int) numero;
}