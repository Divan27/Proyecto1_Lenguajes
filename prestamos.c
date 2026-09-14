#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prestamos.h"
#include "cJSON.h"

/* Guarda los datos minimos del ejemplar dentro de un prestamo. */
typedef struct
{
    char *identificador;
    char *nombre;
} EjemplarPrestamo;

/* Representa un prestamo completo y su estado actual. */
typedef struct
{
    char *identificador;
    char *usuario;
    char *fechaInicio;
    char *fechaEntrega;
    char *fechaDevolucion;
    char *estado;
    EjemplarPrestamo *ejemplares;
    int cantidadEjemplares;
    double monto;
} Prestamo;

/* Mantiene en memoria todos los prestamos cargados del JSON. */
struct GestorPrestamos
{
    Prestamo *prestamos;
    int cantidadPrestamos;
    int siguienteId;
};

static char *duplicarTextoPrestamo(const char *texto);
static void liberarPrestamo(Prestamo *prestamo);
static char *leerArchivoCompletoPrestamos(const char *rutaArchivo);
static int fechaValida(const char *fecha);
static long fechaAOrdinal(const char *fecha);
static int diasEntreFechas(const char *inicio, const char *fin);
/* Indica si dos rangos de fechas se cruzan entre si. */
static int intervalosTraslapan(
    const char *inicio1,
    const char *fin1,
    const char *inicio2,
    const char *fin2);
/* Revisa que el ejemplar no este ocupado por otro prestamo en esas fechas. */
static int ejemplarDisponible(
    const GestorPrestamos *gestor,
    const char *identificadorEjemplar,
    const char *fechaInicio,
    const char *fechaEntrega);
static int identificadorRepetido(
    char **identificadores,
    int indiceActual);
static Prestamo *buscarPrestamo(
    GestorPrestamos *gestor,
    const char *identificadorPrestamo);
static int agregarPrestamo(
    GestorPrestamos *gestor,
    Prestamo *prestamo);
/* Selecciona las tarifas segun la duracion pactada del prestamo. */
static int calcularTarifas(
    int duracion,
    int *tarifaDia,
    int *tarifaTardia);

GestorPrestamos *crearGestorPrestamos(void)
{
    GestorPrestamos *gestor = malloc(sizeof(GestorPrestamos));

    if (gestor == NULL)
    {
        return NULL;
    }

    gestor->prestamos = NULL;
    gestor->cantidadPrestamos = 0;
    gestor->siguienteId = 1;

    return gestor;
}

static char *duplicarTextoPrestamo(const char *texto)
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

static int esBisiesto(int anio)
{
    return (
        (anio % 400 == 0) ||
        (anio % 4 == 0 && anio % 100 != 0));
}

static int diasMes(int mes, int anio)
{
    static const int dias[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

    if (mes == 2 && esBisiesto(anio))
    {
        return 29;
    }

    return dias[mes - 1];
}

/* Verifica formato AAAA-MM-DD y que la fecha realmente exista. */
static int fechaValida(const char *fecha)
{
    int anio;
    int mes;
    int dia;
    char extra;

    if (fecha == NULL || strlen(fecha) != 10)
    {
        return 0;
    }

    if (sscanf(fecha, "%4d-%2d-%2d%c", &anio, &mes, &dia, &extra) != 3)
    {
        return 0;
    }

    if (anio < 1 || mes < 1 || mes > 12)
    {
        return 0;
    }

    return dia >= 1 && dia <= diasMes(mes, anio);
}

/* Convierte una fecha a un numero de dias para poder compararla. */
static long fechaAOrdinal(const char *fecha)
{
    int anio;
    int mes;
    int dia;
    int i;
    long total = 0;

    sscanf(fecha, "%d-%d-%d", &anio, &mes, &dia);

    total = 365L * (anio - 1);
    total += (anio - 1) / 4;
    total -= (anio - 1) / 100;
    total += (anio - 1) / 400;

    for (i = 1; i < mes; i++)
    {
        total += diasMes(i, anio);
    }

    total += dia;
    return total;
}

static int diasEntreFechas(const char *inicio, const char *fin)
{
    return (int)(fechaAOrdinal(fin) - fechaAOrdinal(inicio));
}

static int intervalosTraslapan(
    const char *inicio1,
    const char *fin1,
    const char *inicio2,
    const char *fin2)
{
    long i1 = fechaAOrdinal(inicio1);
    long f1 = fechaAOrdinal(fin1);
    long i2 = fechaAOrdinal(inicio2);
    long f2 = fechaAOrdinal(fin2);

    return i1 <= f2 && i2 <= f1;
}

static int ejemplarDisponible(
    const GestorPrestamos *gestor,
    const char *identificadorEjemplar,
    const char *fechaInicio,
    const char *fechaEntrega)
{
    int i;
    int j;

    if (gestor == NULL)
    {
        return 0;
    }

    for (i = 0; i < gestor->cantidadPrestamos; i++)
    {
        const Prestamo *prestamo = &gestor->prestamos[i];
        const char *finOcupacion = prestamo->fechaEntrega;

        if (
            strcmp(prestamo->estado, "FINALIZADO") == 0 &&
            prestamo->fechaDevolucion != NULL &&
            strlen(prestamo->fechaDevolucion) > 0)
        {
            finOcupacion = prestamo->fechaDevolucion;
        }

        for (j = 0; j < prestamo->cantidadEjemplares; j++)
        {
            if (
                strcmp(
                    prestamo->ejemplares[j].identificador,
                    identificadorEjemplar) == 0 &&
                intervalosTraslapan(
                    fechaInicio,
                    fechaEntrega,
                    prestamo->fechaInicio,
                    finOcupacion))
            {
                return 0;
            }
        }
    }

    return 1;
}

static int identificadorRepetido(
    char **identificadores,
    int indiceActual)
{
    int i;

    for (i = 0; i < indiceActual; i++)
    {
        if (strcmp(identificadores[i], identificadores[indiceActual]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int agregarPrestamo(
    GestorPrestamos *gestor,
    Prestamo *prestamo)
{
    Prestamo *temporal = realloc(
        gestor->prestamos,
        (gestor->cantidadPrestamos + 1) * sizeof(Prestamo));

    if (temporal == NULL)
    {
        return 0;
    }

    gestor->prestamos = temporal;
    gestor->prestamos[gestor->cantidadPrestamos] = *prestamo;
    gestor->cantidadPrestamos++;

    return 1;
}

int registrarPrestamo(
    GestorPrestamos *gestor,
    const Catalogo *catalogo,
    GestorUsuarios *usuarios,
    const char *identificacionUsuario,
    const char *fechaInicio,
    const char *fechaEntrega,
    char **identificadores,
    int cantidadSolicitada,
    const char *rutaPrestamos,
    const char *rutaUsuarios)
{
    Prestamo nuevo;
    int i;
    int cantidadDisponibles = 0;
    char idPrestamo[40];

    if (
        gestor == NULL || catalogo == NULL || usuarios == NULL ||
        identificacionUsuario == NULL || fechaInicio == NULL ||
        fechaEntrega == NULL || identificadores == NULL ||
        cantidadSolicitada <= 0)
    {
        return 0;
    }

    if (!usuarioExiste(usuarios, identificacionUsuario))
    {
        printf("\nNo existe un usuario con la identificacion indicada.\n");
        return -1;
    }

    if (!fechaValida(fechaInicio) || !fechaValida(fechaEntrega))
    {
        printf("\nLas fechas deben tener formato AAAA-MM-DD y ser validas.\n");
        return -2;
    }

    if (diasEntreFechas(fechaInicio, fechaEntrega) < 0)
    {
        printf("\nLa fecha de entrega no puede ser anterior a la fecha de inicio.\n");
        return -2;
    }

    nuevo.identificador = NULL;
    nuevo.usuario = NULL;
    nuevo.fechaInicio = NULL;
    nuevo.fechaEntrega = NULL;
    nuevo.fechaDevolucion = NULL;
    nuevo.estado = NULL;
    nuevo.ejemplares = NULL;
    nuevo.cantidadEjemplares = 0;
    nuevo.monto = 0.0;

    nuevo.ejemplares = malloc(cantidadSolicitada * sizeof(EjemplarPrestamo));

    if (nuevo.ejemplares == NULL)
    {
        return 0;
    }

    /* Se valida cada ejemplar por separado antes de incluirlo. */
    for (i = 0; i < cantidadSolicitada; i++)
    {
        const char *nombreLibro = NULL;

        if (identificadorRepetido(identificadores, i))
        {
            printf("- %s: identificador repetido, no se agrega.\n", identificadores[i]);
            continue;
        }

        if (!obtenerNombreEjemplar(catalogo, identificadores[i], &nombreLibro))
        {
            printf("- %s: el ejemplar no existe.\n", identificadores[i]);
            continue;
        }

        if (!ejemplarDisponible(
                gestor,
                identificadores[i],
                fechaInicio,
                fechaEntrega))
        {
            printf("- %s: no esta disponible en las fechas indicadas.\n", identificadores[i]);
            continue;
        }

        nuevo.ejemplares[cantidadDisponibles].identificador =
            duplicarTextoPrestamo(identificadores[i]);
        nuevo.ejemplares[cantidadDisponibles].nombre =
            duplicarTextoPrestamo(nombreLibro);

        if (
            nuevo.ejemplares[cantidadDisponibles].identificador == NULL ||
            nuevo.ejemplares[cantidadDisponibles].nombre == NULL)
        {
            int j;
            free(nuevo.ejemplares[cantidadDisponibles].identificador);
            free(nuevo.ejemplares[cantidadDisponibles].nombre);

            for (j = 0; j < cantidadDisponibles; j++)
            {
                free(nuevo.ejemplares[j].identificador);
                free(nuevo.ejemplares[j].nombre);
            }

            free(nuevo.ejemplares);
            return 0;
        }

        cantidadDisponibles++;
    }

    if (cantidadDisponibles == 0)
    {
        free(nuevo.ejemplares);
        printf("\nNo hay ejemplares validos y disponibles para generar el prestamo.\n");
        return -3;
    }

    if (cantidadDisponibles < cantidadSolicitada)
    {
        EjemplarPrestamo *temporal = realloc(
            nuevo.ejemplares,
            cantidadDisponibles * sizeof(EjemplarPrestamo));

        if (temporal != NULL)
        {
            nuevo.ejemplares = temporal;
        }
    }

    /* Se genera un identificador unico para el nuevo prestamo. */
    snprintf(idPrestamo, sizeof(idPrestamo), "PRE-%d", gestor->siguienteId);

    nuevo.identificador = duplicarTextoPrestamo(idPrestamo);
    nuevo.usuario = duplicarTextoPrestamo(identificacionUsuario);
    nuevo.fechaInicio = duplicarTextoPrestamo(fechaInicio);
    nuevo.fechaEntrega = duplicarTextoPrestamo(fechaEntrega);
    nuevo.estado = duplicarTextoPrestamo("ACTIVO");
    nuevo.cantidadEjemplares = cantidadDisponibles;

    if (
        nuevo.identificador == NULL || nuevo.usuario == NULL ||
        nuevo.fechaInicio == NULL || nuevo.fechaEntrega == NULL ||
        nuevo.estado == NULL)
    {
        liberarPrestamo(&nuevo);
        return 0;
    }

    if (!agregarPrestamo(gestor, &nuevo))
    {
        liberarPrestamo(&nuevo);
        return 0;
    }

    /* El usuario queda marcado con un registro asociado al prestamo. */
    gestor->siguienteId++;
    aumentarRegistrosAsociados(usuarios, identificacionUsuario);

    if (!guardarPrestamosJson(gestor, rutaPrestamos))
    {
        printf("\nAdvertencia: no fue posible guardar los prestamos.\n");
    }

    if (!guardarUsuariosJson(usuarios, rutaUsuarios))
    {
        printf("\nAdvertencia: no fue posible actualizar usuarios.json.\n");
    }

    printf("\n=========================================\n");
    printf("        COMPROBANTE DE PRESTAMO\n");
    printf("=========================================\n");
    printf("Prestamo: %s\n", idPrestamo);
    printf("Usuario: %s\n", identificacionUsuario);
    printf("Fecha inicio: %s\n", fechaInicio);
    printf("Fecha entrega: %s\n", fechaEntrega);
    printf("Ejemplares disponibles incluidos:\n");

    for (i = 0; i < cantidadDisponibles; i++)
    {
        printf(
            "  %s - %s\n",
            gestor->prestamos[gestor->cantidadPrestamos - 1].ejemplares[i].identificador,
            gestor->prestamos[gestor->cantidadPrestamos - 1].ejemplares[i].nombre);
    }

    printf("=========================================\n");

    return 1;
}

static Prestamo *buscarPrestamo(
    GestorPrestamos *gestor,
    const char *identificadorPrestamo)
{
    int i;

    if (gestor == NULL || identificadorPrestamo == NULL)
    {
        return NULL;
    }

    for (i = 0; i < gestor->cantidadPrestamos; i++)
    {
        if (strcmp(gestor->prestamos[i].identificador, identificadorPrestamo) == 0)
        {
            return &gestor->prestamos[i];
        }
    }

    return NULL;
}

static int calcularTarifas(
    int duracion,
    int *tarifaDia,
    int *tarifaTardia)
{
    if (duracion <= 0 || tarifaDia == NULL || tarifaTardia == NULL)
    {
        return 0;
    }

    if (duracion <= 7)
    {
        *tarifaDia = 175;
        *tarifaTardia = 100;
    }
    else if (duracion <= 15)
    {
        *tarifaDia = 150;
        *tarifaTardia = 75;
    }
    else
    {
        *tarifaDia = 100;
        *tarifaTardia = 50;
    }

    return 1;
}

int registrarDevolucion(
    GestorPrestamos *gestor,
    const char *identificadorPrestamo,
    const char *fechaDevolucion,
    const char *rutaPrestamos)
{
    Prestamo *prestamo;
    int duracion;
    int diasTardios;
    int tarifaDia;
    int tarifaTardia;
    double monto;
    char *copiaFecha;
    char *copiaEstado;

    if (
        gestor == NULL || identificadorPrestamo == NULL ||
        fechaDevolucion == NULL)
    {
        return 0;
    }

    prestamo = buscarPrestamo(gestor, identificadorPrestamo);

    if (prestamo == NULL)
    {
        printf("\nNo existe un prestamo con ese identificador.\n");
        return -1;
    }

    if (strcmp(prestamo->estado, "FINALIZADO") == 0)
    {
        printf("\nEl prestamo ya fue devuelto anteriormente.\n");
        return -2;
    }

    if (!fechaValida(fechaDevolucion))
    {
        printf("\nLa fecha de devolucion debe tener formato AAAA-MM-DD y ser valida.\n");
        return -3;
    }

    if (diasEntreFechas(prestamo->fechaInicio, fechaDevolucion) < 0)
    {
        printf("\nLa devolucion no puede ser anterior a la fecha de inicio del prestamo.\n");
        return -3;
    }

    /* La duracion incluye tanto el dia inicial como el dia de entrega. */
    duracion = diasEntreFechas(prestamo->fechaInicio, prestamo->fechaEntrega) + 1;
    diasTardios = diasEntreFechas(prestamo->fechaEntrega, fechaDevolucion);

    if (diasTardios < 0)
    {
        diasTardios = 0;
    }

    calcularTarifas(duracion, &tarifaDia, &tarifaTardia);

    /* Total = costo normal del prestamo + recargo por atraso. */
    monto =
        (double)(duracion * tarifaDia) +
        (double)(diasTardios * tarifaTardia);

    copiaFecha = duplicarTextoPrestamo(fechaDevolucion);
    copiaEstado = duplicarTextoPrestamo("FINALIZADO");

    if (copiaFecha == NULL || copiaEstado == NULL)
    {
        free(copiaFecha);
        free(copiaEstado);
        return 0;
    }

    /* Al devolver, el prestamo pasa a FINALIZADO y se guarda la fecha real. */
    free(prestamo->fechaDevolucion);
    free(prestamo->estado);
    prestamo->fechaDevolucion = copiaFecha;
    prestamo->estado = copiaEstado;
    prestamo->monto = monto;

    if (!guardarPrestamosJson(gestor, rutaPrestamos))
    {
        printf("\nAdvertencia: no fue posible guardar la devolucion.\n");
    }

    printf("\n=========================================\n");
    printf("       COMPROBANTE DE DEVOLUCION\n");
    printf("=========================================\n");
    printf("Prestamo: %s\n", prestamo->identificador);
    printf("Usuario: %s\n", prestamo->usuario);
    printf("Fecha inicio: %s\n", prestamo->fechaInicio);
    printf("Fecha entrega: %s\n", prestamo->fechaEntrega);
    printf("Fecha devolucion: %s\n", fechaDevolucion);
    printf("Duracion pactada: %d dia(s)\n", duracion);
    printf("Tarifa por dia: %d\n", tarifaDia);
    printf("Dias tardios: %d\n", diasTardios);
    printf("Tarifa por dia tardio: %d\n", tarifaTardia);
    printf("Monto a cancelar: %.2f\n", monto);
    printf("Estado: FINALIZADO\n");
    printf("=========================================\n");

    return 1;
}

/* Guarda todos los prestamos actuales para mantener persistencia. */
int guardarPrestamosJson(
    const GestorPrestamos *gestor,
    const char *rutaArchivo)
{
    cJSON *raiz;
    cJSON *arreglo;
    char *texto;
    FILE *archivo;
    int i;
    int j;

    if (gestor == NULL || rutaArchivo == NULL)
    {
        return 0;
    }

    raiz = cJSON_CreateObject();
    arreglo = cJSON_CreateArray();

    if (raiz == NULL || arreglo == NULL)
    {
        cJSON_Delete(raiz);
        cJSON_Delete(arreglo);
        return 0;
    }

    cJSON_AddNumberToObject(raiz, "siguienteId", gestor->siguienteId);
    cJSON_AddItemToObject(raiz, "prestamos", arreglo);

    for (i = 0; i < gestor->cantidadPrestamos; i++)
    {
        const Prestamo *prestamo = &gestor->prestamos[i];
        cJSON *item = cJSON_CreateObject();
        cJSON *ejemplares = cJSON_CreateArray();

        if (item == NULL || ejemplares == NULL)
        {
            cJSON_Delete(item);
            cJSON_Delete(ejemplares);
            cJSON_Delete(raiz);
            return 0;
        }

        cJSON_AddStringToObject(item, "identificador", prestamo->identificador);
        cJSON_AddStringToObject(item, "usuario", prestamo->usuario);
        cJSON_AddStringToObject(item, "fechaInicio", prestamo->fechaInicio);
        cJSON_AddStringToObject(item, "fechaEntrega", prestamo->fechaEntrega);
        cJSON_AddStringToObject(item, "estado", prestamo->estado);
        cJSON_AddNumberToObject(item, "monto", prestamo->monto);

        if (prestamo->fechaDevolucion != NULL)
        {
            cJSON_AddStringToObject(item, "fechaDevolucion", prestamo->fechaDevolucion);
        }
        else
        {
            cJSON_AddNullToObject(item, "fechaDevolucion");
        }

        for (j = 0; j < prestamo->cantidadEjemplares; j++)
        {
            cJSON *ejemplar = cJSON_CreateObject();

            if (ejemplar == NULL)
            {
                cJSON_Delete(raiz);
                return 0;
            }

            cJSON_AddStringToObject(
                ejemplar,
                "identificador",
                prestamo->ejemplares[j].identificador);
            cJSON_AddStringToObject(
                ejemplar,
                "nombre",
                prestamo->ejemplares[j].nombre);
            cJSON_AddItemToArray(ejemplares, ejemplar);
        }

        cJSON_AddItemToObject(item, "ejemplares", ejemplares);
        cJSON_AddItemToArray(arreglo, item);
    }

    texto = cJSON_Print(raiz);

    if (texto == NULL)
    {
        cJSON_Delete(raiz);
        return 0;
    }

    archivo = fopen(rutaArchivo, "w");

    if (archivo == NULL)
    {
        free(texto);
        cJSON_Delete(raiz);
        return 0;
    }

    fputs(texto, archivo);
    fclose(archivo);
    free(texto);
    cJSON_Delete(raiz);

    return 1;
}

static char *leerArchivoCompletoPrestamos(const char *rutaArchivo)
{
    FILE *archivo;
    long tamano;
    char *contenido;

    archivo = fopen(rutaArchivo, "rb");

    if (archivo == NULL)
    {
        return NULL;
    }

    fseek(archivo, 0, SEEK_END);
    tamano = ftell(archivo);
    rewind(archivo);

    contenido = malloc((tamano + 1) * sizeof(char));

    if (contenido == NULL)
    {
        fclose(archivo);
        return NULL;
    }

    if (tamano > 0)
    {
        fread(contenido, sizeof(char), tamano, archivo);
    }

    contenido[tamano] = '\0';
    fclose(archivo);

    return contenido;
}

/* Carga los prestamos existentes al iniciar el programa. */
int cargarPrestamosDesdeJson(
    GestorPrestamos *gestor,
    const char *rutaArchivo)
{
    char *contenido;
    cJSON *raiz;
    cJSON *arreglo;
    cJSON *siguienteId;
    int cantidad;
    int i;

    if (gestor == NULL || rutaArchivo == NULL)
    {
        return 0;
    }

    contenido = leerArchivoCompletoPrestamos(rutaArchivo);

    if (contenido == NULL)
    {
        /* La primera ejecucion puede no tener prestamos.json. */
        return 1;
    }

    raiz = cJSON_Parse(contenido);
    free(contenido);

    if (raiz == NULL)
    {
        return 0;
    }

    siguienteId = cJSON_GetObjectItemCaseSensitive(raiz, "siguienteId");
    arreglo = cJSON_GetObjectItemCaseSensitive(raiz, "prestamos");

    if (!cJSON_IsArray(arreglo))
    {
        cJSON_Delete(raiz);
        return 0;
    }

    if (cJSON_IsNumber(siguienteId) && siguienteId->valueint > 0)
    {
        gestor->siguienteId = siguienteId->valueint;
    }

    cantidad = cJSON_GetArraySize(arreglo);

    for (i = 0; i < cantidad; i++)
    {
        cJSON *item = cJSON_GetArrayItem(arreglo, i);
        cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "identificador");
        cJSON *usuario = cJSON_GetObjectItemCaseSensitive(item, "usuario");
        cJSON *inicio = cJSON_GetObjectItemCaseSensitive(item, "fechaInicio");
        cJSON *entrega = cJSON_GetObjectItemCaseSensitive(item, "fechaEntrega");
        cJSON *devolucion = cJSON_GetObjectItemCaseSensitive(item, "fechaDevolucion");
        cJSON *estado = cJSON_GetObjectItemCaseSensitive(item, "estado");
        cJSON *monto = cJSON_GetObjectItemCaseSensitive(item, "monto");
        cJSON *ejemplares = cJSON_GetObjectItemCaseSensitive(item, "ejemplares");
        Prestamo prestamo;
        int j;
        int cantidadEjemplares;

        if (
            !cJSON_IsString(id) || !cJSON_IsString(usuario) ||
            !cJSON_IsString(inicio) || !cJSON_IsString(entrega) ||
            !cJSON_IsString(estado) || !cJSON_IsArray(ejemplares))
        {
            cJSON_Delete(raiz);
            return 0;
        }

        prestamo.identificador = duplicarTextoPrestamo(id->valuestring);
        prestamo.usuario = duplicarTextoPrestamo(usuario->valuestring);
        prestamo.fechaInicio = duplicarTextoPrestamo(inicio->valuestring);
        prestamo.fechaEntrega = duplicarTextoPrestamo(entrega->valuestring);
        prestamo.fechaDevolucion =
            cJSON_IsString(devolucion) ? duplicarTextoPrestamo(devolucion->valuestring) : NULL;
        prestamo.estado = duplicarTextoPrestamo(estado->valuestring);
        prestamo.monto = cJSON_IsNumber(monto) ? monto->valuedouble : 0.0;
        prestamo.ejemplares = NULL;
        prestamo.cantidadEjemplares = 0;

        if (
            prestamo.identificador == NULL || prestamo.usuario == NULL ||
            prestamo.fechaInicio == NULL || prestamo.fechaEntrega == NULL ||
            prestamo.estado == NULL)
        {
            liberarPrestamo(&prestamo);
            cJSON_Delete(raiz);
            return 0;
        }

        cantidadEjemplares = cJSON_GetArraySize(ejemplares);

        if (cantidadEjemplares > 0)
        {
            prestamo.ejemplares = malloc(
                cantidadEjemplares * sizeof(EjemplarPrestamo));

            if (prestamo.ejemplares == NULL)
            {
                liberarPrestamo(&prestamo);
                cJSON_Delete(raiz);
                return 0;
            }
        }

        for (j = 0; j < cantidadEjemplares; j++)
        {
            cJSON *ejemplar = cJSON_GetArrayItem(ejemplares, j);
            cJSON *ejemplarId = cJSON_GetObjectItemCaseSensitive(ejemplar, "identificador");
            cJSON *nombre = cJSON_GetObjectItemCaseSensitive(ejemplar, "nombre");

            if (!cJSON_IsString(ejemplarId) || !cJSON_IsString(nombre))
            {
                int k;
                for (k = 0; k < j; k++)
                {
                    free(prestamo.ejemplares[k].identificador);
                    free(prestamo.ejemplares[k].nombre);
                }
                free(prestamo.ejemplares);
                prestamo.ejemplares = NULL;
                liberarPrestamo(&prestamo);
                cJSON_Delete(raiz);
                return 0;
            }

            prestamo.ejemplares[j].identificador =
                duplicarTextoPrestamo(ejemplarId->valuestring);
            prestamo.ejemplares[j].nombre =
                duplicarTextoPrestamo(nombre->valuestring);

            if (
                prestamo.ejemplares[j].identificador == NULL ||
                prestamo.ejemplares[j].nombre == NULL)
            {
                int k;
                free(prestamo.ejemplares[j].identificador);
                free(prestamo.ejemplares[j].nombre);
                for (k = 0; k < j; k++)
                {
                    free(prestamo.ejemplares[k].identificador);
                    free(prestamo.ejemplares[k].nombre);
                }
                free(prestamo.ejemplares);
                prestamo.ejemplares = NULL;
                liberarPrestamo(&prestamo);
                cJSON_Delete(raiz);
                return 0;
            }
        }

        prestamo.cantidadEjemplares = cantidadEjemplares;

        if (!agregarPrestamo(gestor, &prestamo))
        {
            liberarPrestamo(&prestamo);
            cJSON_Delete(raiz);
            return 0;
        }
    }

    cJSON_Delete(raiz);
    return 1;
}

int ejemplarEstaDisponible(
    const GestorPrestamos *gestor,
    const char *identificadorEjemplar)
{
    int i;
    int j;

    if (gestor == NULL || identificadorEjemplar == NULL)
    {
        return 0;
    }

    for (i = 0; i < gestor->cantidadPrestamos; i++)
    {
        const Prestamo *prestamo = &gestor->prestamos[i];

        if (strcmp(prestamo->estado, "FINALIZADO") == 0)
        {
            continue;
        }

        for (j = 0; j < prestamo->cantidadEjemplares; j++)
        {
            if (strcmp(
                    prestamo->ejemplares[j].identificador,
                    identificadorEjemplar) == 0)
            {
                return 0;
            }
        }
    }

    return 1;
}

static void liberarPrestamo(Prestamo *prestamo){
    int i;

    if (prestamo == NULL)
    {
        return;
    }

    free(prestamo->identificador);
    free(prestamo->usuario);
    free(prestamo->fechaInicio);
    free(prestamo->fechaEntrega);
    free(prestamo->fechaDevolucion);
    free(prestamo->estado);

    for (i = 0; i < prestamo->cantidadEjemplares; i++)
    {
        free(prestamo->ejemplares[i].identificador);
        free(prestamo->ejemplares[i].nombre);
    }

    free(prestamo->ejemplares);

    prestamo->identificador = NULL;
    prestamo->usuario = NULL;
    prestamo->fechaInicio = NULL;
    prestamo->fechaEntrega = NULL;
    prestamo->fechaDevolucion = NULL;
    prestamo->estado = NULL;
    prestamo->ejemplares = NULL;
    prestamo->cantidadEjemplares = 0;
}

void destruirGestorPrestamos(
    GestorPrestamos *gestor)
{
    int i;

    if (gestor == NULL)
    {
        return;
    }

    for (i = 0; i < gestor->cantidadPrestamos; i++)
    {
        liberarPrestamo(&gestor->prestamos[i]);
    }

    free(gestor->prestamos);
    free(gestor);
}
