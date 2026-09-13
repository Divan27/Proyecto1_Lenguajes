#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usuarios.h"
#include "configuracion.h"
#include "cJSON.h"

typedef struct
{
    char *identificacion;
    char *nombre;
    char *direccion;

    int registrosAsociados;

} Usuario;

struct GestorUsuarios
{
    Usuario *usuarios;

    int cantidadUsuarios;
};

/* ===================================== */
/* PROTOTIPOS INTERNOS                   */
/* ===================================== */

static char *duplicarTextoUsuario(
    const char *texto);

static int buscarIndiceUsuario(
    const GestorUsuarios *gestor,
    const char *identificacion);

static void liberarUsuario(
    Usuario *usuario);

static char *leerArchivoUsuarios(
    const char *rutaArchivo);

/* ===================================== */
/* CREAR GESTOR                          */
/* ===================================== */

GestorUsuarios *crearGestorUsuarios(void)
{
    GestorUsuarios *gestor;

    gestor = malloc(
        sizeof(GestorUsuarios));

    if (gestor == NULL)
    {
        return NULL;
    }

    gestor->usuarios = NULL;

    gestor->cantidadUsuarios = 0;

    return gestor;
}

/* ===================================== */
/* DUPLICAR TEXTO                        */
/* ===================================== */

static char *duplicarTextoUsuario(
    const char *texto)
{
    char *copia;

    size_t longitud;

    if (texto == NULL)
    {
        return NULL;
    }

    longitud = strlen(texto);

    copia = malloc(
        (longitud + 1) * sizeof(char));

    if (copia == NULL)
    {
        return NULL;
    }

    strcpy(
        copia,
        texto);

    return copia;
}

/* ===================================== */
/* BUSCAR USUARIO                        */
/* ===================================== */

static int buscarIndiceUsuario(
    const GestorUsuarios *gestor,
    const char *identificacion)
{
    int i;

    if (
        gestor == NULL ||
        identificacion == NULL)
    {
        return -1;
    }

    for (
        i = 0;
        i < gestor->cantidadUsuarios;
        i++)
    {
        if (
            strcmp(
                gestor
                    ->usuarios[i]
                    .identificacion,
                identificacion) == 0)
        {
            return i;
        }
    }

    return -1;
}

/* ===================================== */
/* COMPROBAR EXISTENCIA                  */
/* ===================================== */

int usuarioExiste(
    const GestorUsuarios *gestor,
    const char *identificacion)
{
    return (
        buscarIndiceUsuario(
            gestor,
            identificacion) != -1);
}

/* ===================================== */
/* CREAR USUARIO                         */
/* ===================================== */

int crearUsuario(
    GestorUsuarios *gestor,
    const char *identificacion,
    const char *nombre,
    const char *direccion)
{
    Usuario nuevoUsuario;

    Usuario *temporal;

    if (
        gestor == NULL ||
        identificacion == NULL ||
        nombre == NULL ||
        direccion == NULL)
    {
        return 0;
    }

    if (
        strlen(identificacion) == 0 ||
        strlen(nombre) == 0 ||
        strlen(direccion) == 0)
    {
        return 0;
    }

    if (
        usuarioExiste(
            gestor,
            identificacion))
    {
        return -1;
    }

    nuevoUsuario.identificacion =
        duplicarTextoUsuario(
            identificacion);

    nuevoUsuario.nombre =
        duplicarTextoUsuario(
            nombre);

    nuevoUsuario.direccion =
        duplicarTextoUsuario(
            direccion);

    nuevoUsuario.registrosAsociados = 0;

    if (
        nuevoUsuario.identificacion == NULL ||
        nuevoUsuario.nombre == NULL ||
        nuevoUsuario.direccion == NULL)
    {
        liberarUsuario(
            &nuevoUsuario);

        return 0;
    }

    temporal = realloc(
        gestor->usuarios,
        (
            gestor->cantidadUsuarios + 1) *
            sizeof(Usuario));

    if (temporal == NULL)
    {
        liberarUsuario(
            &nuevoUsuario);

        return 0;
    }

    gestor->usuarios = temporal;

    gestor
        ->usuarios[gestor->cantidadUsuarios] = nuevoUsuario;

    gestor->cantidadUsuarios++;

    return 1;
}

/* ===================================== */
/* MOSTRAR USUARIOS                      */
/* ===================================== */

void mostrarUsuarios(
    const GestorUsuarios *gestor)
{
    int i;

    printf("\n");
    printf("=========================================\n");
    printf("              USUARIOS\n");
    printf("=========================================\n");

    if (
        gestor == NULL ||
        gestor->cantidadUsuarios == 0)
    {
        printf(
            "No existen usuarios registrados.\n");

        return;
    }

    for (
        i = 0;
        i < gestor->cantidadUsuarios;
        i++)
    {
        const Usuario *usuario;

        usuario =
            &gestor->usuarios[i];

        printf("\n");

        printf(
            "Identificacion: %s\n",
            usuario->identificacion);

        printf(
            "Nombre: %s\n",
            usuario->nombre);

        printf(
            "Direccion: %s\n",
            usuario->direccion);

        printf(
            "Registros asociados: %d\n",
            usuario->registrosAsociados);

        printf(
            "-----------------------------------------\n");
    }

    printf(
        "\nTotal de usuarios: %d\n",
        gestor->cantidadUsuarios);
}

/* ===================================== */
/* MODIFICAR USUARIO                     */
/* ===================================== */

int modificarUsuario(
    GestorUsuarios *gestor,
    const char *identificacion,
    const char *nuevoNombre,
    const char *nuevaDireccion)
{
    int indice;

    char *nombreTemporal;

    char *direccionTemporal;

    indice = buscarIndiceUsuario(
        gestor,
        identificacion);

    if (indice == -1)
    {
        return -1;
    }

    if (
        nuevoNombre == NULL ||
        nuevaDireccion == NULL ||
        strlen(nuevoNombre) == 0 ||
        strlen(nuevaDireccion) == 0)
    {
        return 0;
    }

    nombreTemporal =
        duplicarTextoUsuario(
            nuevoNombre);

    direccionTemporal =
        duplicarTextoUsuario(
            nuevaDireccion);

    if (
        nombreTemporal == NULL ||
        direccionTemporal == NULL)
    {
        free(nombreTemporal);
        free(direccionTemporal);

        return 0;
    }

    free(
        gestor
            ->usuarios[indice]
            .nombre);

    free(
        gestor
            ->usuarios[indice]
            .direccion);

    gestor
        ->usuarios[indice]
        .nombre = nombreTemporal;

    gestor
        ->usuarios[indice]
        .direccion = direccionTemporal;

    return 1;
}

/* ===================================== */
/* ELIMINAR USUARIO                      */
/* ===================================== */

int eliminarUsuario(
    GestorUsuarios *gestor,
    const char *identificacion)
{
    int indice;

    int i;

    Usuario *temporal;

    indice = buscarIndiceUsuario(
        gestor,
        identificacion);

    if (indice == -1)
    {
        return -1;
    }

    if (
        gestor
            ->usuarios[indice]
            .registrosAsociados > 0)
    {
        return -2;
    }

    liberarUsuario(
        &gestor->usuarios[indice]);

    for (
        i = indice;
        i < gestor->cantidadUsuarios - 1;
        i++)
    {
        gestor->usuarios[i] =
            gestor->usuarios[i + 1];
    }

    gestor->cantidadUsuarios--;

    if (
        gestor->cantidadUsuarios == 0)
    {
        free(
            gestor->usuarios);

        gestor->usuarios = NULL;

        return 1;
    }

    temporal = realloc(
        gestor->usuarios,
        gestor->cantidadUsuarios *
            sizeof(Usuario));

    if (temporal != NULL)
    {
        gestor->usuarios = temporal;
    }

    return 1;
}

/* ===================================== */
/* REGISTROS ASOCIADOS                   */
/* ===================================== */

int aumentarRegistrosAsociados(
    GestorUsuarios *gestor,
    const char *identificacion)
{
    int indice;

    indice = buscarIndiceUsuario(
        gestor,
        identificacion);

    if (indice == -1)
    {
        return 0;
    }

    gestor
        ->usuarios[indice]
        .registrosAsociados++;

    return 1;
}

int disminuirRegistrosAsociados(
    GestorUsuarios *gestor,
    const char *identificacion)
{
    int indice;

    indice = buscarIndiceUsuario(
        gestor,
        identificacion);

    if (indice == -1)
    {
        return 0;
    }

    if (
        gestor
            ->usuarios[indice]
            .registrosAsociados > 0)
    {
        gestor
            ->usuarios[indice]
            .registrosAsociados--;
    }

    return 1;
}

/* ===================================== */
/* LIBERAR USUARIO                       */
/* ===================================== */

static void liberarUsuario(
    Usuario *usuario)
{
    if (usuario == NULL)
    {
        return;
    }

    free(
        usuario->identificacion);

    free(
        usuario->nombre);

    free(
        usuario->direccion);

    usuario->identificacion = NULL;

    usuario->nombre = NULL;

    usuario->direccion = NULL;
}

/* ===================================== */
/* GUARDAR JSON                          */
/* ===================================== */

int guardarUsuariosJson(
    const GestorUsuarios *gestor,
    const char *rutaArchivo)
{
    cJSON *raiz;

    cJSON *usuariosJson;

    char *textoJson;

    FILE *archivo;

    int i;

    raiz = cJSON_CreateObject();

    if (raiz == NULL)
    {
        return 0;
    }

    usuariosJson =
        cJSON_CreateArray();

    if (usuariosJson == NULL)
    {
        cJSON_Delete(
            raiz);

        return 0;
    }

    cJSON_AddItemToObject(
        raiz,
        "usuarios",
        usuariosJson);

    for (
        i = 0;
        i < gestor->cantidadUsuarios;
        i++)
    {
        const Usuario *usuario;

        cJSON *usuarioJson;

        usuario =
            &gestor->usuarios[i];

        usuarioJson =
            cJSON_CreateObject();

        if (usuarioJson == NULL)
        {
            cJSON_Delete(
                raiz);

            return 0;
        }

        cJSON_AddStringToObject(
            usuarioJson,
            "identificacion",
            usuario->identificacion);

        cJSON_AddStringToObject(
            usuarioJson,
            "nombre",
            usuario->nombre);

        cJSON_AddStringToObject(
            usuarioJson,
            "direccion",
            usuario->direccion);

        cJSON_AddNumberToObject(
            usuarioJson,
            "registrosAsociados",
            usuario->registrosAsociados);

        cJSON_AddItemToArray(
            usuariosJson,
            usuarioJson);
    }

    textoJson =
        cJSON_Print(
            raiz);

    if (textoJson == NULL)
    {
        cJSON_Delete(
            raiz);

        return 0;
    }

    archivo = fopen(
        rutaArchivo,
        "w");

    if (archivo == NULL)
    {
        cJSON_free(
            textoJson);

        cJSON_Delete(
            raiz);

        return 0;
    }

    fprintf(
        archivo,
        "%s",
        textoJson);

    fclose(
        archivo);

    cJSON_free(
        textoJson);

    cJSON_Delete(
        raiz);

    return 1;
}

/* ===================================== */
/* LEER ARCHIVO COMPLETO                 */
/* ===================================== */

static char *leerArchivoUsuarios(
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
        fclose(
            archivo);

        return NULL;
    }

    fread(
        contenido,
        sizeof(char),
        tamanio,
        archivo);

    contenido[tamanio] = '\0';

    fclose(
        archivo);

    return contenido;
}

/* ===================================== */
/* CARGAR DESDE JSON                     */
/* ===================================== */

int cargarUsuariosDesdeJson(
    GestorUsuarios *gestor,
    const char *rutaArchivo)
{
    char *contenido;

    cJSON *raiz;

    cJSON *usuariosJson;

    cJSON *usuarioJson;

    contenido =
        leerArchivoUsuarios(
            rutaArchivo);

    /*
        Si todavía no existe usuarios.json,
        no se considera un error.
    */
    if (contenido == NULL)
    {
        return 1;
    }

    raiz =
        cJSON_Parse(
            contenido);

    free(
        contenido);

    if (raiz == NULL)
    {
        return 0;
    }

    usuariosJson =
        cJSON_GetObjectItemCaseSensitive(
            raiz,
            "usuarios");

    if (
        !cJSON_IsArray(
            usuariosJson))
    {
        cJSON_Delete(
            raiz);

        return 0;
    }

    cJSON_ArrayForEach(
        usuarioJson,
        usuariosJson)
    {
        cJSON *identificacion;

        cJSON *nombre;

        cJSON *direccion;

        cJSON *registros;

        Usuario nuevoUsuario;

        Usuario *temporal;

        identificacion =
            cJSON_GetObjectItemCaseSensitive(
                usuarioJson,
                "identificacion");

        nombre =
            cJSON_GetObjectItemCaseSensitive(
                usuarioJson,
                "nombre");

        direccion =
            cJSON_GetObjectItemCaseSensitive(
                usuarioJson,
                "direccion");

        registros =
            cJSON_GetObjectItemCaseSensitive(
                usuarioJson,
                "registrosAsociados");

        if (
            !cJSON_IsString(identificacion) ||
            !cJSON_IsString(nombre) ||
            !cJSON_IsString(direccion))
        {
            continue;
        }

        nuevoUsuario.identificacion =
            duplicarTextoUsuario(
                identificacion->valuestring);

        nuevoUsuario.nombre =
            duplicarTextoUsuario(
                nombre->valuestring);

        nuevoUsuario.direccion =
            duplicarTextoUsuario(
                direccion->valuestring);

        if (
            cJSON_IsNumber(
                registros))
        {
            nuevoUsuario.registrosAsociados =
                registros->valueint;
        }
        else
        {
            nuevoUsuario.registrosAsociados = 0;
        }

        if (
            nuevoUsuario.identificacion == NULL ||
            nuevoUsuario.nombre == NULL ||
            nuevoUsuario.direccion == NULL)
        {
            liberarUsuario(
                &nuevoUsuario);

            continue;
        }

        temporal = realloc(
            gestor->usuarios,
            (
                gestor->cantidadUsuarios + 1) *
                sizeof(Usuario));

        if (temporal == NULL)
        {
            liberarUsuario(
                &nuevoUsuario);

            continue;
        }

        gestor->usuarios = temporal;

        gestor
            ->usuarios[gestor->cantidadUsuarios] = nuevoUsuario;

        gestor->cantidadUsuarios++;
    }

    cJSON_Delete(
        raiz);

    return 1;
}

/* ===================================== */
/* DESTRUIR GESTOR                       */
/* ===================================== */

void destruirGestorUsuarios(
    GestorUsuarios *gestor)
{
    int i;

    if (gestor == NULL)
    {
        return;
    }

    for (
        i = 0;
        i < gestor->cantidadUsuarios;
        i++)
    {
        liberarUsuario(
            &gestor->usuarios[i]);
    }

    free(
        gestor->usuarios);

    free(
        gestor);
}