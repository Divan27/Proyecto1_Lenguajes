#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "prestamos.h"
#include "catalogo.h"
#include "configuracion.h"
#include "cJSON.h"

typedef struct
{
    char *identificador;

} Ejemplar;

typedef struct
{
    int id;

    char *nombre;
    char *autor;
    int anioPublicacion;
    char *genero;
    char *resumen;

    int cantidad;

    Ejemplar *ejemplares;

} Libro;

typedef struct
{
    char **mensajes;
    int cantidad;

} ReporteCarga;

struct Catalogo
{
    Libro *libros;

    int cantidadLibros;

    int siguienteId;
};

/* ============================= */
/* PROTOTIPOS INTERNOS           */
/* ============================= */

static char *duplicarTexto(const char *texto);

static char *leerLineaArchivo(FILE *archivo);

static int separarCampos(
    char *linea,
    char ***campos,
    int cantidadEsperada);

static void liberarCampos(
    char **campos,
    int cantidad);

static int convertirEntero(
    const char *texto,
    int *resultado);

static int textosIguales(
    const char *texto1,
    const char *texto2);

static int existeLibro(
    const Catalogo *catalogo,
    const char *nombre);

static char *crearIdentificadorEjemplar(
    int idLibro,
    int numeroEjemplar);

static int crearEjemplares(
    Libro *libro);

static void liberarLibro(
    Libro *libro);

static int agregarLibro(
    Catalogo *catalogo,
    Libro *nuevoLibro);

static void agregarRechazo(
    ReporteCarga *reporte,
    int numeroLinea,
    const char *motivo,
    const char *contenido);

static void mostrarReporte(
    const ReporteCarga *reporte);

static void destruirReporte(
    ReporteCarga *reporte);

static int guardarCatalogoJson(
    const Catalogo *catalogo,
    const char *rutaArchivo);

static char *leerArchivoCompleto(
    const char *rutaArchivo);

static int contieneTexto(
    const char *texto,
    const char *busqueda);

/* ============================= */
/* contieneTexto                */
/* ============================= */

static int contieneTexto(
    const char *texto,
    const char *busqueda)
{
    size_t longitudTexto;
    size_t longitudBusqueda;
    size_t i;
    size_t j;

    if (texto == NULL || busqueda == NULL)
    {
        return 0;
    }

    longitudTexto = strlen(texto);
    longitudBusqueda = strlen(busqueda);

    if (longitudBusqueda == 0 || longitudBusqueda > longitudTexto)
    {
        return 0;
    }

    for (i = 0; i <= longitudTexto - longitudBusqueda; i++)
    {
        for (j = 0; j < longitudBusqueda; j++)
        {
            if (tolower((unsigned char)texto[i + j]) !=
                tolower((unsigned char)busqueda[j]))
            {
                break;
            }
        }

        if (j == longitudBusqueda)
        {
            return 1;
        }
    }

    return 0;
}

/* ============================= */
/* CREAR CATALOGO                */
/* ============================= */

Catalogo *crearCatalogo(void)
{
    Catalogo *catalogo;

    catalogo = malloc(sizeof(Catalogo));

    if (catalogo == NULL)
    {
        return NULL;
    }

    catalogo->libros = NULL;
    catalogo->cantidadLibros = 0;
    catalogo->siguienteId = 1;

    return catalogo;
}

/* ============================= */
/* DUPLICAR TEXTO DINAMICAMENTE  */
/* ============================= */

static char *duplicarTexto(const char *texto)
{
    char *copia;
    size_t longitud;

    if (texto == NULL)
    {
        return NULL;
    }

    longitud = strlen(texto);

    copia = malloc((longitud + 1) * sizeof(char));

    if (copia == NULL)
    {
        return NULL;
    }

    strcpy(copia, texto);

    return copia;
}

/* ============================= */
/* LEER LINEA DINAMICA           */
/* ============================= */

static char *leerLineaArchivo(FILE *archivo)
{
    char *linea;

    int capacidad = TAMANIO_BLOQUE_LECTURA;
    int cantidad = 0;

    int caracter;

    linea = malloc(capacidad * sizeof(char));

    if (linea == NULL)
    {
        return NULL;
    }

    while (
        (caracter = fgetc(archivo)) != '\n' &&
        caracter != EOF)
    {
        if (cantidad + 1 >= capacidad)
        {
            int nuevaCapacidad;

            char *temporal;

            nuevaCapacidad =
                capacidad + TAMANIO_BLOQUE_LECTURA;

            temporal = realloc(
                linea,
                nuevaCapacidad * sizeof(char));

            if (temporal == NULL)
            {
                free(linea);

                return NULL;
            }

            linea = temporal;
            capacidad = nuevaCapacidad;
        }

        if (caracter != '\r')
        {
            linea[cantidad] = (char)caracter;

            cantidad++;
        }
    }

    if (cantidad == 0 && caracter == EOF)
    {
        free(linea);

        return NULL;
    }

    linea[cantidad] = '\0';

    return linea;
}

/* ============================= */
/* SEPARAR LOS 6 CAMPOS          */
/* ============================= */

static int separarCampos(
    char *linea,
    char ***campos,
    int cantidadEsperada)
{
    char **resultado;

    int cantidad = 0;

    char *inicio;
    char *actual;

    resultado = malloc(
        cantidadEsperada * sizeof(char *));

    if (resultado == NULL)
    {
        return 0;
    }

    inicio = linea;
    actual = linea;

    while (1)
    {
        if (*actual == '#' || *actual == '\0')
        {
            size_t longitud;

            char *campo;

            longitud = actual - inicio;

            if (cantidad >= cantidadEsperada)
            {
                liberarCampos(
                    resultado,
                    cantidad);

                return 0;
            }

            campo = malloc(
                (longitud + 1) * sizeof(char));

            if (campo == NULL)
            {
                liberarCampos(
                    resultado,
                    cantidad);

                return 0;
            }

            memcpy(
                campo,
                inicio,
                longitud);

            campo[longitud] = '\0';

            resultado[cantidad] = campo;

            cantidad++;

            if (*actual == '\0')
            {
                break;
            }

            inicio = actual + 1;
        }

        actual++;
    }

    if (cantidad != cantidadEsperada)
    {
        liberarCampos(
            resultado,
            cantidad);

        return 0;
    }

    *campos = resultado;

    return 1;
}

/* ============================= */
/* LIBERAR CAMPOS                */
/* ============================= */

static void liberarCampos(
    char **campos,
    int cantidad)
{
    int i;

    if (campos == NULL)
    {
        return;
    }

    for (i = 0; i < cantidad; i++)
    {
        free(campos[i]);
    }

    free(campos);
}

/* ============================= */
/* CONVERTIR A ENTERO            */
/* ============================= */

static int convertirEntero(
    const char *texto,
    int *resultado)
{
    char *final;

    long numero;

    if (
        texto == NULL ||
        resultado == NULL)
    {
        return 0;
    }

    numero = strtol(
        texto,
        &final,
        10);

    if (
        final == texto ||
        *final != '\0')
    {
        return 0;
    }

    *resultado = (int)numero;

    return 1;
}

/* ============================= */
/* COMPARAR TEXTOS               */
/* ============================= */

static int textosIguales(
    const char *texto1,
    const char *texto2)
{
    int posicion = 0;

    if (
        texto1 == NULL ||
        texto2 == NULL)
    {
        return 0;
    }

    while (
        texto1[posicion] != '\0' &&
        texto2[posicion] != '\0')
    {
        if (
            tolower((unsigned char)texto1[posicion]) !=
            tolower((unsigned char)texto2[posicion]))
        {
            return 0;
        }

        posicion++;
    }

    return (
        texto1[posicion] == '\0' &&
        texto2[posicion] == '\0');
}

/* ============================= */
/* BUSCAR DUPLICADO              */
/* ============================= */

static int existeLibro(
    const Catalogo *catalogo,
    const char *nombre)
{
    int i;

    for (
        i = 0;
        i < catalogo->cantidadLibros;
        i++)
    {
        if (
            textosIguales(
                catalogo->libros[i].nombre,
                nombre))
        {
            return 1;
        }
    }

    return 0;
}

/* ============================= */
/* IDENTIFICADOR DE EJEMPLAR     */
/* ============================= */

static char *crearIdentificadorEjemplar(
    int idLibro,
    int numeroEjemplar)
{
    int longitud;

    char *identificador;

    longitud = snprintf(
        NULL,
        0,
        "LIB-%d-EJ-%d",
        idLibro,
        numeroEjemplar);

    identificador = malloc(
        (longitud + 1) * sizeof(char));

    if (identificador == NULL)
    {
        return NULL;
    }

    snprintf(
        identificador,
        longitud + 1,
        "LIB-%d-EJ-%d",
        idLibro,
        numeroEjemplar);

    return identificador;
}

/* ============================= */
/* CREAR EJEMPLARES              */
/* ============================= */

static int crearEjemplares(
    Libro *libro)
{
    int i;

    libro->ejemplares = malloc(
        libro->cantidad * sizeof(Ejemplar));

    if (libro->ejemplares == NULL)
    {
        return 0;
    }

    for (i = 0; i < libro->cantidad; i++)
    {
        libro->ejemplares[i].identificador =
            crearIdentificadorEjemplar(
                libro->id,
                i + 1);

        if (
            libro->ejemplares[i].identificador ==
            NULL)
        {
            int j;

            for (j = 0; j < i; j++)
            {
                free(
                    libro
                        ->ejemplares[j]
                        .identificador);
            }

            free(libro->ejemplares);

            libro->ejemplares = NULL;

            return 0;
        }
    }

    return 1;
}

/* ============================= */
/* LIBERAR LIBRO                 */
/* ============================= */

static void liberarLibro(
    Libro *libro)
{
    int i;

    if (libro == NULL)
    {
        return;
    }

    free(libro->nombre);
    free(libro->autor);
    free(libro->genero);
    free(libro->resumen);

    for (i = 0; i < libro->cantidad; i++)
    {
        free(
            libro
                ->ejemplares[i]
                .identificador);
    }

    free(libro->ejemplares);

    libro->nombre = NULL;
    libro->autor = NULL;
    libro->genero = NULL;
    libro->resumen = NULL;
    libro->ejemplares = NULL;
}

/* ============================= */
/* AGREGAR LIBRO AL ARREGLO      */
/* ============================= */

static int agregarLibro(
    Catalogo *catalogo,
    Libro *nuevoLibro)
{
    Libro *temporal;

    temporal = realloc(
        catalogo->libros,
        (
            catalogo->cantidadLibros + 1) *
            sizeof(Libro));

    if (temporal == NULL)
    {
        return 0;
    }

    catalogo->libros = temporal;

    catalogo
        ->libros[catalogo->cantidadLibros] =
        *nuevoLibro;

    catalogo->cantidadLibros++;

    return 1;
}

/* ============================= */
/* AGREGAR RECHAZO               */
/* ============================= */

static void agregarRechazo(
    ReporteCarga *reporte,
    int numeroLinea,
    const char *motivo,
    const char *contenido)
{
    char **temporal;

    char *mensaje;

    int longitud;

    temporal = realloc(
        reporte->mensajes,
        (
            reporte->cantidad + 1) *
            sizeof(char *));

    if (temporal == NULL)
    {
        return;
    }

    reporte->mensajes = temporal;

    longitud = snprintf(
        NULL,
        0,
        "Linea %d | %s | %s",
        numeroLinea,
        motivo,
        contenido);

    mensaje = malloc(
        (longitud + 1) * sizeof(char));

    if (mensaje == NULL)
    {
        return;
    }

    snprintf(
        mensaje,
        longitud + 1,
        "Linea %d | %s | %s",
        numeroLinea,
        motivo,
        contenido);

    reporte
        ->mensajes[reporte->cantidad] =
        mensaje;

    reporte->cantidad++;
}

/* ============================= */
/* MOSTRAR REPORTE               */
/* ============================= */

static void mostrarReporte(
    const ReporteCarga *reporte)
{
    int i;

    printf("\n");
    printf("=========================================\n");
    printf("       REPORTE DE NO PROCESADOS\n");
    printf("=========================================\n");

    if (reporte->cantidad == 0)
    {
        printf(
            "Todos los registros fueron procesados correctamente.\n");

        return;
    }

    for (
        i = 0;
        i < reporte->cantidad;
        i++)
    {
        printf(
            "%s\n",
            reporte->mensajes[i]);
    }

    printf(
        "\nTotal no procesados: %d\n",
        reporte->cantidad);
}

/* ============================= */
/* DESTRUIR REPORTE              */
/* ============================= */

static void destruirReporte(
    ReporteCarga *reporte)
{
    int i;

    for (
        i = 0;
        i < reporte->cantidad;
        i++)
    {
        free(reporte->mensajes[i]);
    }

    free(reporte->mensajes);

    reporte->mensajes = NULL;
    reporte->cantidad = 0;
}

/* ============================= */
/* CARGA POR LOTE                */
/* ============================= */

void incluirCatalogoPorLote(
    Catalogo *catalogo,
    const char *rutaLote,
    const char *rutaJson)
{
    FILE *archivo;

    char *linea;

    int numeroLinea = 0;
    int procesados = 0;

    ReporteCarga reporte;

    reporte.mensajes = NULL;
    reporte.cantidad = 0;

    archivo = fopen(
        rutaLote,
        "r");

    if (archivo == NULL)
    {
        printf(
            "\nNo fue posible abrir el archivo:\n%s\n",
            rutaLote);

        return;
    }

    while (
        (linea = leerLineaArchivo(archivo)) != NULL)
    {
        char **campos = NULL;

        int anio;
        int cantidad;

        Libro nuevoLibro;

        numeroLinea++;

        if (strlen(linea) == 0)
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Linea vacia",
                linea);

            free(linea);

            continue;
        }

        if (
            !separarCampos(
                linea,
                &campos,
                6))
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Formato incorrecto",
                linea);

            free(linea);

            continue;
        }

        if (
            strlen(campos[0]) == 0 ||
            strlen(campos[1]) == 0 ||
            strlen(campos[2]) == 0 ||
            strlen(campos[3]) == 0 ||
            strlen(campos[4]) == 0 ||
            strlen(campos[5]) == 0)
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Hay campos vacios",
                linea);

            liberarCampos(
                campos,
                6);

            free(linea);

            continue;
        }

        if (
            !convertirEntero(
                campos[2],
                &anio))
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Anio invalido",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        if (
            anio < ANIO_MINIMO ||
            anio > ANIO_MAXIMO)
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Anio fuera de rango",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        if (
            !convertirEntero(
                campos[5],
                &cantidad) ||
            cantidad < CANTIDAD_MINIMA)
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "Cantidad invalida",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        if (
            existeLibro(
                catalogo,
                campos[0]))
        {
            agregarRechazo(
                &reporte,
                numeroLinea,
                "El libro ya existe",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        nuevoLibro.id =
            catalogo->siguienteId;

        nuevoLibro.nombre =
            duplicarTexto(campos[0]);

        nuevoLibro.autor =
            duplicarTexto(campos[1]);

        nuevoLibro.anioPublicacion =
            anio;

        nuevoLibro.genero =
            duplicarTexto(campos[3]);

        nuevoLibro.resumen =
            duplicarTexto(campos[4]);

        nuevoLibro.cantidad =
            cantidad;

        nuevoLibro.ejemplares = NULL;

        if (
            nuevoLibro.nombre == NULL ||
            nuevoLibro.autor == NULL ||
            nuevoLibro.genero == NULL ||
            nuevoLibro.resumen == NULL)
        {
            liberarLibro(&nuevoLibro);

            agregarRechazo(
                &reporte,
                numeroLinea,
                "Error de memoria",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        if (
            !crearEjemplares(
                &nuevoLibro))
        {
            liberarLibro(&nuevoLibro);

            agregarRechazo(
                &reporte,
                numeroLinea,
                "No fue posible crear ejemplares",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        if (
            !agregarLibro(
                catalogo,
                &nuevoLibro))
        {
            liberarLibro(&nuevoLibro);

            agregarRechazo(
                &reporte,
                numeroLinea,
                "No fue posible agregar al catalogo",
                linea);

            liberarCampos(campos, 6);
            free(linea);

            continue;
        }

        catalogo->siguienteId++;

        procesados++;

        liberarCampos(
            campos,
            6);

        free(linea);
    }

    fclose(archivo);

    if (
        guardarCatalogoJson(
            catalogo,
            rutaJson))
    {
        printf("\nCarga finalizada correctamente.\n");

        printf(
            "Registros procesados: %d\n",
            procesados);
    }
    else
    {
        printf(
            "\nError al guardar catalogo.json\n");
    }

    mostrarReporte(
        &reporte);

    destruirReporte(
        &reporte);
}

/* ============================= */
/* GUARDAR JSON                  */
/* ============================= */

static int guardarCatalogoJson(
    const Catalogo *catalogo,
    const char *rutaArchivo)
{
    cJSON *raiz;

    cJSON *librosJson;

    char *textoJson;

    FILE *archivo;

    int i;

    raiz = cJSON_CreateObject();

    if (raiz == NULL)
    {
        return 0;
    }

    librosJson = cJSON_CreateArray();

    if (librosJson == NULL)
    {
        cJSON_Delete(raiz);

        return 0;
    }

    cJSON_AddItemToObject(
        raiz,
        "libros",
        librosJson);

    for (
        i = 0;
        i < catalogo->cantidadLibros;
        i++)
    {
        const Libro *libro;

        cJSON *libroJson;
        cJSON *ejemplaresJson;

        int j;

        libro = &catalogo->libros[i];

        libroJson =
            cJSON_CreateObject();

        cJSON_AddNumberToObject(
            libroJson,
            "id",
            libro->id);

        cJSON_AddStringToObject(
            libroJson,
            "nombre",
            libro->nombre);

        cJSON_AddStringToObject(
            libroJson,
            "autor",
            libro->autor);

        cJSON_AddNumberToObject(
            libroJson,
            "anioPublicacion",
            libro->anioPublicacion);

        cJSON_AddStringToObject(
            libroJson,
            "genero",
            libro->genero);

        cJSON_AddStringToObject(
            libroJson,
            "resumen",
            libro->resumen);

        cJSON_AddNumberToObject(
            libroJson,
            "cantidad",
            libro->cantidad);

        ejemplaresJson =
            cJSON_CreateArray();

        for (
            j = 0;
            j < libro->cantidad;
            j++)
        {
            cJSON *ejemplarJson;

            ejemplarJson =
                cJSON_CreateObject();

            cJSON_AddStringToObject(
                ejemplarJson,
                "identificador",
                libro
                    ->ejemplares[j]
                    .identificador);

            cJSON_AddItemToArray(
                ejemplaresJson,
                ejemplarJson);
        }

        cJSON_AddItemToObject(
            libroJson,
            "ejemplares",
            ejemplaresJson);

        cJSON_AddItemToArray(
            librosJson,
            libroJson);
    }

    textoJson =
        cJSON_Print(raiz);

    if (textoJson == NULL)
    {
        cJSON_Delete(raiz);

        return 0;
    }

    archivo = fopen(
        rutaArchivo,
        "w");

    if (archivo == NULL)
    {
        cJSON_free(textoJson);
        cJSON_Delete(raiz);

        return 0;
    }

    fprintf(
        archivo,
        "%s",
        textoJson);

    fclose(archivo);

    cJSON_free(textoJson);

    cJSON_Delete(raiz);

    return 1;
}

/* ============================= */
/* LEER ARCHIVO COMPLETO         */
/* ============================= */

static char *leerArchivoCompleto(
    const char *rutaArchivo)
{
    FILE *archivo;

    long tamanio;

    char *contenido;

    archivo = fopen(
        rutaArchivo,
        "rb");

    if (archivo == NULL)
    {
        return NULL;
    }

    fseek(
        archivo,
        0,
        SEEK_END);

    tamanio = ftell(
        archivo);

    rewind(
        archivo);

    contenido = malloc(
        (tamanio + 1) * sizeof(char));

    if (contenido == NULL)
    {
        fclose(archivo);

        return NULL;
    }

    fread(
        contenido,
        sizeof(char),
        tamanio,
        archivo);

    contenido[tamanio] = '\0';

    fclose(archivo);

    return contenido;
}

/* ============================= */
/* CARGAR JSON AL INICIAR        */
/* ============================= */

int cargarCatalogoDesdeJson(
    Catalogo *catalogo,
    const char *rutaArchivo)
{
    char *contenido;

    cJSON *raiz;
    cJSON *librosJson;
    cJSON *libroJson;

    contenido =
        leerArchivoCompleto(
            rutaArchivo);

    if (contenido == NULL)
    {
        return 1;
    }

    raiz =
        cJSON_Parse(contenido);

    free(contenido);

    if (raiz == NULL)
    {
        return 0;
    }

    librosJson =
        cJSON_GetObjectItemCaseSensitive(
            raiz,
            "libros");

    if (
        !cJSON_IsArray(librosJson))
    {
        cJSON_Delete(raiz);

        return 0;
    }

    cJSON_ArrayForEach(
        libroJson,
        librosJson)
    {
        Libro libro;

        cJSON *id;
        cJSON *nombre;
        cJSON *autor;
        cJSON *anio;
        cJSON *genero;
        cJSON *resumen;
        cJSON *cantidad;

        id =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "id");

        nombre =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "nombre");

        autor =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "autor");

        anio =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "anioPublicacion");

        genero =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "genero");

        resumen =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "resumen");

        cantidad =
            cJSON_GetObjectItemCaseSensitive(
                libroJson,
                "cantidad");

        if (
            !cJSON_IsNumber(id) ||
            !cJSON_IsString(nombre) ||
            !cJSON_IsString(autor) ||
            !cJSON_IsNumber(anio) ||
            !cJSON_IsString(genero) ||
            !cJSON_IsString(resumen) ||
            !cJSON_IsNumber(cantidad))
        {
            continue;
        }

        libro.id = id->valueint;

        libro.nombre =
            duplicarTexto(
                nombre->valuestring);

        libro.autor =
            duplicarTexto(
                autor->valuestring);

        libro.anioPublicacion =
            anio->valueint;

        libro.genero =
            duplicarTexto(
                genero->valuestring);

        libro.resumen =
            duplicarTexto(
                resumen->valuestring);

        libro.cantidad =
            cantidad->valueint;

        libro.ejemplares = NULL;

        if (
            !crearEjemplares(
                &libro))
        {
            liberarLibro(&libro);

            continue;
        }

        if (
            !agregarLibro(
                catalogo,
                &libro))
        {
            liberarLibro(&libro);

            continue;
        }

        if (
            libro.id >=
            catalogo->siguienteId)
        {
            catalogo->siguienteId =
                libro.id + 1;
        }
    }

    cJSON_Delete(raiz);

    return 1;
}

/* ============================= */
/* MOSTRAR CATALOGO              */
/* ============================= */

void mostrarCatalogo(
    const Catalogo *catalogo)
{
    int i;

    printf("\n");
    printf("=========================================\n");
    printf("             CATALOGO\n");
    printf("=========================================\n");

    if (
        catalogo->cantidadLibros == 0)
    {
        printf(
            "El catalogo se encuentra vacio.\n");

        return;
    }

    for (
        i = 0;
        i < catalogo->cantidadLibros;
        i++)
    {
        const Libro *libro;

        int j;

        libro =
            &catalogo->libros[i];

        printf("\n");
        printf(
            "Libro #%d\n",
            libro->id);

        printf(
            "Nombre: %s\n",
            libro->nombre);

        printf(
            "Autor: %s\n",
            libro->autor);

        printf(
            "Anio de publicacion: %d\n",
            libro->anioPublicacion);

        printf(
            "Genero: %s\n",
            libro->genero);

        printf(
            "Resumen: %s\n",
            libro->resumen);

        printf(
            "Cantidad: %d\n",
            libro->cantidad);

        printf(
            "Ejemplares:\n");

        for (
            j = 0;
            j < libro->cantidad;
            j++)
        {
            printf(
                "   - %s\n",
                libro
                    ->ejemplares[j]
                    .identificador);
        }

        printf(
            "-----------------------------------------\n");
    }

    printf(
        "\nTotal de producciones bibliograficas: %d\n",
        catalogo->cantidadLibros);
}

/* ============================= */
/* DESTRUIR CATALOGO             */
/* ============================= */


/* ============================= */
/* BUSCAR EJEMPLAR               */
/* ============================= */

/* Busca un ejemplar por ID y devuelve el nombre del libro al que pertenece. */
int obtenerNombreEjemplar(
    const Catalogo *catalogo,
    const char *identificador,
    const char **nombreLibro)
{
    int i;
    int j;

    if (
        catalogo == NULL ||
        identificador == NULL ||
        nombreLibro == NULL)
    {
        return 0;
    }

    for (i = 0; i < catalogo->cantidadLibros; i++)
    {
        for (j = 0; j < catalogo->libros[i].cantidad; j++)
        {
            if (
                strcmp(
                    catalogo->libros[i].ejemplares[j].identificador,
                    identificador) == 0)
            {
                *nombreLibro = catalogo->libros[i].nombre;
                return 1;
            }
        }
    }

    *nombreLibro = NULL;
    return 0;
}

void buscarCatalogoSimple(
    const Catalogo *catalogo,
    const struct GestorPrestamos *gestorPrestamos,
    const char *texto)
{
    int i;
    int encontrados = 0;

    if (catalogo == NULL || gestorPrestamos == NULL || texto == NULL)
    {
        return;
    }

    if (strlen(texto) == 0)
    {
        printf("\nDebe ingresar un texto para buscar.\n");
        return;
    }

    printf("\n");
    printf("=========================================\n");
    printf("            BUSQUEDA SIMPLE\n");
    printf("=========================================\n");
    printf("Texto buscado: %s\n", texto);

    for (i = 0; i < catalogo->cantidadLibros; i++)
    {
        const Libro *libro = &catalogo->libros[i];
        int coincide;
        int j;

        coincide =
            contieneTexto(libro->nombre, texto) ||
            contieneTexto(libro->autor, texto) ||
            contieneTexto(libro->resumen, texto);

        if (!coincide)
        {
            continue;
        }

        for (j = 0; j < libro->cantidad; j++)
        {
            const char *identificador =
                libro->ejemplares[j].identificador;

            printf("\n-----------------------------------------\n");

            printf(
                "Identificador: %s\n",
                identificador);

            printf(
                "Nombre: %s\n",
                libro->nombre);

            printf(
                "Resumen: %s\n",
                libro->resumen);

            printf(
                "Estado: %s\n",
                ejemplarEstaDisponible(
                    gestorPrestamos,
                    identificador)
                    ? "DISPONIBLE"
                    : "NO DISPONIBLE");

            encontrados++;
        }
    }

    if (encontrados == 0)
    {
        printf(
            "\nNo se encontraron ejemplares que coincidan.\n");
    }
    else
    {
        printf("\n=========================================\n");

        printf(
            "Ejemplares encontrados: %d\n",
            encontrados);
    }
}

void destruirCatalogo(
    Catalogo *catalogo)
{
    int i;

    if (catalogo == NULL)
    {
        return;
    }

    for (
        i = 0;
        i < catalogo->cantidadLibros;
        i++)
    {
        liberarLibro(
            &catalogo->libros[i]);
    }

    free(
        catalogo->libros);

    free(
        catalogo);
}
