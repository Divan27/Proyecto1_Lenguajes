#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "historial.h"
#include "cJSON.h"

/* ========================================= */
/* PROTOTIPOS INTERNOS                       */
/* ========================================= */

static char *leerArchivoCompletoHistorial(
    const char *rutaArchivo);

static int convertirFechaNumero(
    const char *fecha);

static int obtenerEstadoPrestamo(
    const char *estadoGuardado,
    const char *fechaEntrega);

static const char *textoEstado(
    int estado);

static int entregaTardia(
    const char *fechaEntrega,
    const char *fechaDevolucion);

/* Estados internos */

#define ESTADO_ACTIVO 1
#define ESTADO_VENCIDO 2
#define ESTADO_FINALIZADO 3

/* ========================================= */
/* VALIDAR FECHA                             */
/* ========================================= */

int fechaValida(
    const char *fecha)
{
    int anio;
    int mes;
    int dia;

    char extra;

    int cantidad;

    if (fecha == NULL)
    {
        return 0;
    }

    cantidad = sscanf(
        fecha,
        "%d-%d-%d%c",
        &anio,
        &mes,
        &dia,
        &extra);

    /*
        Deben leerse exactamente
        año, mes y día.
    */
    if (cantidad != 3)
    {
        return 0;
    }

    if (anio < 1)
    {
        return 0;
    }

    if (
        mes < 1 ||
        mes > 12)
    {
        return 0;
    }

    if (
        dia < 1 ||
        dia > 31)
    {
        return 0;
    }

    /*
        Meses con máximo 30 días
    */
    if (
        (
            mes == 4 ||
            mes == 6 ||
            mes == 9 ||
            mes == 11) &&
        dia > 30)
    {
        return 0;
    }

    /*
        Febrero
    */
    if (mes == 2)
    {
        int esBisiesto;

        esBisiesto =
            ((anio % 4 == 0 && anio % 100 != 0) ||
             anio % 400 == 0);

        if (
            esBisiesto &&
            dia > 29)
        {
            return 0;
        }

        if (
            !esBisiesto &&
            dia > 28)
        {
            return 0;
        }
    }

    return 1;
}

/* ========================================= */
/* CONVERTIR FECHA A NUMERO                  */
/* ========================================= */

static int convertirFechaNumero(
    const char *fecha)
{
    int anio;
    int mes;
    int dia;

    if (
        !fechaValida(fecha))
    {
        return -1;
    }

    sscanf(
        fecha,
        "%d-%d-%d",
        &anio,
        &mes,
        &dia);

    /*
        Ejemplo:

        2026-09-13

        pasa a:

        20260913
    */

    return (
        anio * 10000 +
        mes * 100 +
        dia);
}

/* ========================================= */
/* LEER JSON COMPLETO                        */
/* ========================================= */

static char *leerArchivoCompletoHistorial(
    const char *rutaArchivo)
{
    FILE *archivo;

    long tamanio;

    char *contenido;

    size_t cantidadLeida;

    archivo = fopen(
        rutaArchivo,
        "rb");

    if (archivo == NULL)
    {
        return NULL;
    }

    if (
        fseek(
            archivo,
            0,
            SEEK_END) != 0)
    {
        fclose(archivo);

        return NULL;
    }

    tamanio = ftell(
        archivo);

    if (tamanio < 0)
    {
        fclose(archivo);

        return NULL;
    }

    rewind(
        archivo);

    contenido = malloc(
        (tamanio + 1) * sizeof(char));

    if (contenido == NULL)
    {
        fclose(archivo);

        return NULL;
    }

    cantidadLeida = fread(
        contenido,
        sizeof(char),
        tamanio,
        archivo);

    contenido[cantidadLeida] =
        '\0';

    fclose(
        archivo);

    return contenido;
}

/* ========================================= */
/* DETERMINAR ESTADO                         */
/* ========================================= */

static int obtenerEstadoPrestamo(
    const char *estadoGuardado,
    const char *fechaEntrega)
{
    time_t tiempoActual;

    struct tm *fechaActual;

    int hoy;

    int entrega;

    if (
        estadoGuardado != NULL &&
        strcmp(
            estadoGuardado,
            "FINALIZADO") == 0)
    {
        return ESTADO_FINALIZADO;
    }

    entrega =
        convertirFechaNumero(
            fechaEntrega);

    if (entrega == -1)
    {
        return ESTADO_ACTIVO;
    }

    tiempoActual =
        time(NULL);

    fechaActual =
        localtime(
            &tiempoActual);

    if (fechaActual == NULL)
    {
        return ESTADO_ACTIVO;
    }

    hoy =
        (fechaActual->tm_year + 1900) * 10000;

    hoy +=
        (fechaActual->tm_mon + 1) * 100;

    hoy +=
        fechaActual->tm_mday;

    if (hoy > entrega)
    {
        return ESTADO_VENCIDO;
    }

    return ESTADO_ACTIVO;
}

/* ========================================= */
/* TEXTO DEL ESTADO                          */
/* ========================================= */

static const char *textoEstado(
    int estado)
{
    switch (estado)
    {
    case ESTADO_ACTIVO:

        return "ACTIVO";

    case ESTADO_VENCIDO:

        return "VENCIDO";

    case ESTADO_FINALIZADO:

        return "FINALIZADO";

    default:

        return "DESCONOCIDO";
    }
}

/* ========================================= */
/* DETERMINAR ENTREGA TARDIA                 */
/* ========================================= */

static int entregaTardia(
    const char *fechaEntrega,
    const char *fechaDevolucion)
{
    int entrega;
    int devolucion;

    entrega =
        convertirFechaNumero(
            fechaEntrega);

    devolucion =
        convertirFechaNumero(
            fechaDevolucion);

    if (
        entrega == -1 ||
        devolucion == -1)
    {
        return 0;
    }

    return (
        devolucion > entrega);
}

/* ========================================= */
/* MOSTRAR HISTORIAL                         */
/* ========================================= */

void mostrarHistorialPrestamos(
    const char *rutaArchivo,
    const char *fechaInicio,
    const char *fechaFin)
{
    char *contenido;

    cJSON *raiz;
    cJSON *prestamos;
    cJSON *prestamo;

    int inicio;
    int fin;

    int encontrados = 0;

    if (
        !fechaValida(fechaInicio) ||
        !fechaValida(fechaFin))
    {
        printf(
            "\nLas fechas ingresadas no son validas.\n");

        printf(
            "Utilice el formato AAAA-MM-DD.\n");

        return;
    }

    inicio =
        convertirFechaNumero(
            fechaInicio);

    fin =
        convertirFechaNumero(
            fechaFin);

    if (inicio > fin)
    {
        printf(
            "\nLa fecha inicial no puede ser posterior a la fecha final.\n");

        return;
    }

    contenido =
        leerArchivoCompletoHistorial(
            rutaArchivo);

    if (contenido == NULL)
    {
        printf(
            "\nNo fue posible abrir el archivo de prestamos.\n");

        return;
    }

    raiz =
        cJSON_Parse(
            contenido);

    free(
        contenido);

    if (raiz == NULL)
    {
        printf(
            "\nEl archivo de prestamos contiene un JSON invalido.\n");

        return;
    }

    prestamos =
        cJSON_GetObjectItemCaseSensitive(
            raiz,
            "prestamos");

    if (
        !cJSON_IsArray(
            prestamos))
    {
        printf(
            "\nNo existe una lista valida de prestamos.\n");

        cJSON_Delete(
            raiz);

        return;
    }

    printf("\n");
    printf("=========================================\n");
    printf("        HISTORIAL DE PRESTAMOS\n");
    printf("=========================================\n");
    printf(
        "Fecha inicial: %s\n",
        fechaInicio);

    printf(
        "Fecha final:   %s\n",
        fechaFin);

    printf("=========================================\n");

    cJSON_ArrayForEach(
        prestamo,
        prestamos)
    {
        cJSON *identificador;
        cJSON *usuario;
        cJSON *fechaEntrega;
        cJSON *estado;
        cJSON *fechaDevolucion;
        cJSON *ejemplares;

        int fechaEntregaNumero;

        int estadoPrestamo;

        identificador =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "identificador");

        usuario =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "usuario");

        fechaEntrega =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "fechaEntrega");

        estado =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "estado");

        fechaDevolucion =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "fechaDevolucion");

        ejemplares =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "ejemplares");

        /*
            Validamos los campos indispensables.
        */

        if (
            !cJSON_IsString(
                identificador) ||
            !cJSON_IsString(
                usuario) ||
            !cJSON_IsString(
                fechaEntrega))
        {
            continue;
        }

        fechaEntregaNumero =
            convertirFechaNumero(
                fechaEntrega->valuestring);

        if (
            fechaEntregaNumero == -1)
        {
            continue;
        }

        /*
            El requisito dice considerar
            la FECHA DE ENTREGA.
        */

        if (
            fechaEntregaNumero < inicio ||
            fechaEntregaNumero > fin)
        {
            continue;
        }

        encontrados++;

        if (
            cJSON_IsString(
                estado))
        {
            estadoPrestamo =
                obtenerEstadoPrestamo(
                    estado->valuestring,
                    fechaEntrega->valuestring);
        }
        else
        {
            estadoPrestamo =
                obtenerEstadoPrestamo(
                    NULL,
                    fechaEntrega->valuestring);
        }

        printf("\n");
        printf(
            "Prestamo: %s\n",
            identificador->valuestring);

        printf(
            "Usuario: %s\n",
            usuario->valuestring);

        printf(
            "Fecha de entrega: %s\n",
            fechaEntrega->valuestring);

        printf(
            "Estado: %s\n",
            textoEstado(
                estadoPrestamo));

        printf(
            "Ejemplares:\n");

        if (
            cJSON_IsArray(
                ejemplares))
        {
            cJSON *ejemplar;

            cJSON_ArrayForEach(
                ejemplar,
                ejemplares)
            {
                cJSON *idEjemplar;

                cJSON *nombreEjemplar;

                idEjemplar =
                    cJSON_GetObjectItemCaseSensitive(
                        ejemplar,
                        "identificador");

                nombreEjemplar =
                    cJSON_GetObjectItemCaseSensitive(
                        ejemplar,
                        "nombre");

                if (
                    cJSON_IsString(
                        idEjemplar) &&
                    cJSON_IsString(
                        nombreEjemplar))
                {
                    printf(
                        "   - %s | %s\n",
                        idEjemplar->valuestring,
                        nombreEjemplar->valuestring);
                }
            }
        }

        /*
            Solamente los préstamos finalizados
            muestran si hubo entrega tardía.
        */

        if (
            estadoPrestamo ==
            ESTADO_FINALIZADO)
        {
            if (
                cJSON_IsString(
                    fechaDevolucion))
            {
                printf(
                    "Entrega tardia: %s\n",
                    entregaTardia(
                        fechaEntrega->valuestring,
                        fechaDevolucion->valuestring)
                        ? "SI"
                        : "NO");
            }
            else
            {
                printf(
                    "Entrega tardia: NO\n");
            }
        }

        printf(
            "-----------------------------------------\n");
    }

    if (encontrados == 0)
    {
        printf(
            "\nNo existen prestamos con fecha de entrega dentro del rango indicado.\n");
    }
    else
    {
        printf(
            "\nTotal de prestamos encontrados: %d\n",
            encontrados);
    }

    cJSON_Delete(
        raiz);
}