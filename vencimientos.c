#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vencimientos.h"
#include "cJSON.h"
#include "configuracion.h"

/* ========================================= */
/* PROTOTIPOS INTERNOS                       */
/* ========================================= */

static char *leerArchivoVencimientos(
    const char *rutaArchivo);

static int convertirFecha(
    const char *fechaTexto,
    struct tm *fecha);

static int calcularDiasDiferencia(
    const char *fechaEntrega);

static void mostrarEjemplaresVencimiento(
    cJSON *ejemplares);

/* ========================================= */
/* LEER ARCHIVO COMPLETO                     */
/* ========================================= */

static char *leerArchivoVencimientos(
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
        fclose(
            archivo);

        return NULL;
    }

    tamanio = ftell(
        archivo);

    if (tamanio < 0)
    {
        fclose(
            archivo);

        return NULL;
    }

    rewind(
        archivo);

    contenido = malloc(
        (tamanio + 1) * sizeof(char));

    if (contenido == NULL)
    {
        fclose(
            archivo);

        return NULL;
    }

    cantidadLeida = fread(
        contenido,
        sizeof(char),
        tamanio,
        archivo);

    contenido[cantidadLeida] = '\0';

    fclose(
        archivo);

    return contenido;
}

/* ========================================= */
/* CONVERTIR TEXTO A FECHA                   */
/* ========================================= */

static int convertirFecha(
    const char *fechaTexto,
    struct tm *fecha)
{
    int anio;
    int mes;
    int dia;

    char extra;

    int cantidad;

    if (
        fechaTexto == NULL ||
        fecha == NULL)
    {
        return 0;
    }

    cantidad = sscanf(
        fechaTexto,
        "%d-%d-%d%c",
        &anio,
        &mes,
        &dia,
        &extra);

    if (cantidad != 3)
    {
        return 0;
    }

    if (
        mes < 1 ||
        mes > 12 ||
        dia < 1 ||
        dia > 31)
    {
        return 0;
    }

    /*
        Inicializamos todos los campos
        de la estructura en cero.
    */

    memset(
        fecha,
        0,
        sizeof(struct tm));

    fecha->tm_year =
        anio - 1900;

    fecha->tm_mon =
        mes - 1;

    fecha->tm_mday =
        dia;

    /*
        Mediodia evita algunos problemas
        relacionados con cambios de hora.
    */

    fecha->tm_hour = 12;

    /*
        Permitimos que mktime determine
        automáticamente horario de verano.
    */

    fecha->tm_isdst = -1;

    return 1;
}

/* ========================================= */
/* CALCULAR DIFERENCIA EN DIAS               */
/* ========================================= */

static int calcularDiasDiferencia(
    const char *fechaEntrega)
{
    time_t tiempoActual;

    time_t tiempoEntrega;

    struct tm fechaActual;

    struct tm fechaEntregaStruct;

    struct tm *punteroActual;

    double diferencia;

    int dias;

    /*
        Obtener fecha y hora del sistema.
    */

    tiempoActual =
        time(NULL);

    punteroActual =
        localtime(
            &tiempoActual);

    if (punteroActual == NULL)
    {
        return 999999;
    }

    /*
        Copiamos la estructura porque
        localtime utiliza memoria interna.
    */

    fechaActual =
        *punteroActual;

    /*
        Trabajamos únicamente con la fecha.
    */

    fechaActual.tm_hour = 12;
    fechaActual.tm_min = 0;
    fechaActual.tm_sec = 0;
    fechaActual.tm_isdst = -1;

    tiempoActual =
        mktime(
            &fechaActual);

    if (
        !convertirFecha(
            fechaEntrega,
            &fechaEntregaStruct))
    {
        return 999999;
    }

    tiempoEntrega =
        mktime(
            &fechaEntregaStruct);

    if (
        tiempoEntrega == (time_t)-1 ||
        tiempoActual == (time_t)-1)
    {
        return 999999;
    }

    diferencia =
        difftime(
            tiempoEntrega,
            tiempoActual);

    dias =
        (int)(diferencia /
              (60 * 60 * 24));

    return dias;
}

/* ========================================= */
/* MOSTRAR EJEMPLARES                        */
/* ========================================= */

static void mostrarEjemplaresVencimiento(
    cJSON *ejemplares)
{
    cJSON *ejemplar;

    if (
        !cJSON_IsArray(
            ejemplares))
    {
        printf(
            "Ejemplares: informacion no disponible.\n");

        return;
    }

    printf(
        "Ejemplares:\n");

    cJSON_ArrayForEach(
        ejemplar,
        ejemplares)
    {
        cJSON *identificador;

        cJSON *nombre;

        identificador =
            cJSON_GetObjectItemCaseSensitive(
                ejemplar,
                "identificador");

        nombre =
            cJSON_GetObjectItemCaseSensitive(
                ejemplar,
                "nombre");

        if (
            cJSON_IsString(identificador) &&
            cJSON_IsString(nombre))
        {
            printf(
                "   - %s | %s\n",
                identificador->valuestring,
                nombre->valuestring);
        }
    }
}

/* ========================================= */
/* MOSTRAR VENCIMIENTOS                      */
/* ========================================= */

void mostrarVencimientosPrestamos(
    const char *rutaArchivo)
{
    char *contenido;

    cJSON *raiz;

    cJSON *prestamos;

    cJSON *prestamo;

    int encontrados = 0;

    time_t ahora;

    struct tm *fechaSistema;

    contenido =
        leerArchivoVencimientos(
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
            "\nEl archivo prestamos.json contiene datos invalidos.\n");

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

    /*
        Mostrar fecha actual utilizada
        para el cálculo.
    */

    ahora =
        time(NULL);

    fechaSistema =
        localtime(
            &ahora);

    printf("\n");
    printf("=========================================\n");
    printf("      VENCIMIENTO DE PRESTAMOS\n");
    printf("=========================================\n");

    if (fechaSistema != NULL)
    {
        printf(
            "Fecha del sistema: %04d-%02d-%02d\n",
            fechaSistema->tm_year + 1900,
            fechaSistema->tm_mon + 1,
            fechaSistema->tm_mday);
    }

    printf("=========================================\n");

    cJSON_ArrayForEach(
        prestamo,
        prestamos)
    {
        cJSON *identificador;

        cJSON *usuario;

        cJSON *fechaEntrega;

        cJSON *estado;

        cJSON *ejemplares;

        int diasRestantes;

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

        ejemplares =
            cJSON_GetObjectItemCaseSensitive(
                prestamo,
                "ejemplares");

        /*
            Campos indispensables.
        */

        if (
            !cJSON_IsString(identificador) ||
            !cJSON_IsString(usuario) ||
            !cJSON_IsString(fechaEntrega))
        {
            continue;
        }

        /*
            Los préstamos finalizados
            ya no pueden vencer.
        */

        if (
            cJSON_IsString(estado) &&
            strcmp(
                estado->valuestring,
                "FINALIZADO") == 0)
        {
            continue;
        }

        diasRestantes =
            calcularDiasDiferencia(
                fechaEntrega->valuestring);

        /*
            Valor utilizado en caso
            de una fecha inválida.
        */

        if (
            diasRestantes == 999999)
        {
            continue;
        }

        /*
            Si faltan más de cinco días,
            no pertenece al reporte.
        */

        if (
            diasRestantes >
            DIAS_PROXIMO_VENCIMIENTO)
        {
            continue;
        }

        encontrados++;

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

        /*
            Una cantidad negativa significa
            que la fecha ya pasó.
        */

        if (diasRestantes < 0)
        {
            printf(
                "Estatus: VENCIDO\n");

            printf(
                "Dias de atraso: %d\n",
                -diasRestantes);
        }
        else
        {
            printf(
                "Estatus: PROXIMO A VENCER\n");

            if (diasRestantes == 0)
            {
                printf(
                    "Vence: HOY\n");
            }
            else
            {
                printf(
                    "Dias restantes: %d\n",
                    diasRestantes);
            }
        }

        mostrarEjemplaresVencimiento(
            ejemplares);

        printf(
            "-----------------------------------------\n");
    }

    if (encontrados == 0)
    {
        printf(
            "\nNo existen prestamos vencidos "
            "o proximos a vencer.\n");
    }
    else
    {
        printf(
            "\nTotal encontrados: %d\n",
            encontrados);
    }

    cJSON_Delete(
        raiz);
}