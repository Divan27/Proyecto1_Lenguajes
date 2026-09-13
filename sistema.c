#include <stdio.h>
#include <stdlib.h>

#include "sistema.h"
#include "menu.h"
#include "entrada.h"
#include "configuracion.h"
#include "catalogo.h"
#include "usuarios.h"

struct Sistema
{
    int activo;

    Catalogo *catalogo;

    GestorUsuarios *gestorUsuarios;
};

static void ejecutarMenuPrincipal(Sistema *sistema);

static void ejecutarMenuOperativo(
    Sistema *sistema);

static void ejecutarGestionCatalogo(
    Catalogo *catalogo);

static void ejecutarMenuGeneral(void);

static void ejecutarGestionUsuarios(
    GestorUsuarios *gestor);

Sistema *crearSistema(void)
{
    Sistema *nuevoSistema;

    nuevoSistema = malloc(sizeof(Sistema));

    if (nuevoSistema == NULL)
    {
        printf("Error reservando memoria para el sistema.\n");
        return NULL;
    }

    nuevoSistema->activo = SISTEMA_ACTIVO;

    /* ========================= */
    /* CREAR CATALOGO            */
    /* ========================= */

    nuevoSistema->catalogo = crearCatalogo();

    if (nuevoSistema->catalogo == NULL)
    {
        printf("Error creando el catalogo.\n");

        free(nuevoSistema);

        return NULL;
    }

    if (
        !cargarCatalogoDesdeJson(
            nuevoSistema->catalogo,
            ARCHIVO_CATALOGO))
    {
        printf(
            "Advertencia: no fue posible cargar catalogo.json\n");
    }

    /* ========================= */
    /* CREAR GESTOR DE USUARIOS  */
    /* ========================= */

    nuevoSistema->gestorUsuarios =
        crearGestorUsuarios();

    if (nuevoSistema->gestorUsuarios == NULL)
    {
        printf(
            "Error creando el gestor de usuarios.\n");

        destruirCatalogo(
            nuevoSistema->catalogo);

        free(
            nuevoSistema);

        return NULL;
    }

    if (
        !cargarUsuariosDesdeJson(
            nuevoSistema->gestorUsuarios,
            ARCHIVO_USUARIOS))
    {
        printf(
            "Advertencia: no fue posible cargar usuarios.json\n");
    }

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
            "Seleccione una opcion: ");

        switch (opcion)
        {
        case OPCION_MENU_OPERATIVAS:

            ejecutarMenuOperativo(sistema);

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

static void ejecutarMenuOperativo(
    Sistema *sistema)
{
    int opcion = 0;

    while (
        opcion !=
        OPCION_VOLVER_OPERATIVAS)
    {
        mostrarMenuOperativo();

        opcion = leerEntero(
            "Seleccione una opcion: ");

        switch (opcion)
        {
        case 1:

            ejecutarGestionCatalogo(
                sistema->catalogo);

            break;

        case 2:

            ejecutarGestionUsuarios(
                sistema->gestorUsuarios);

            break;

        case 3:

            mostrarOpcionNoImplementada(
                "Historial de prestamos");

            break;

        case 4:

            mostrarOpcionNoImplementada(
                "Vencimiento de prestamos");

            break;

        case 5:

            mostrarOpcionNoImplementada(
                "Estadisticas");

            break;

        case OPCION_VOLVER_OPERATIVAS:

            printf(
                "\nVolviendo al menu principal...\n");

            break;

        default:

            printf(
                "\nOpcion invalida.\n");

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
            "Seleccione una opcion: ");

        switch (opcion)
        {
        case 1:

            mostrarOpcionNoImplementada(
                "Busqueda simple");

            break;

        case 2:

            mostrarOpcionNoImplementada(
                "Busqueda avanzada");

            break;

        case 3:

            mostrarOpcionNoImplementada(
                "Prestamo de ejemplares");

            break;

        case 4:

            mostrarOpcionNoImplementada(
                "Devolucion de ejemplar");

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

static void ejecutarGestionCatalogo(
    Catalogo *catalogo)
{
    int opcion = 0;

    while (
        opcion !=
        OPCION_CATALOGO_VOLVER)
    {
        mostrarMenuCatalogo();

        opcion = leerEntero(
            "Seleccione una opcion: ");

        switch (opcion)
        {
        case OPCION_CATALOGO_CARGAR_LOTE:
        {
            char *ruta;

            printf(
                "\nIndique la ruta del archivo: ");

            ruta =
                leerLineaDinamica();

            if (ruta == NULL)
            {
                printf(
                    "No fue posible leer la ruta.\n");

                break;
            }

            incluirCatalogoPorLote(
                catalogo,
                ruta,
                ARCHIVO_CATALOGO);

            free(ruta);

            break;
        }

        case OPCION_CATALOGO_VER:

            mostrarCatalogo(
                catalogo);

            break;

        case OPCION_CATALOGO_VOLVER:

            printf(
                "\nVolviendo a Opciones Operativas...\n");

            break;

        default:

            printf(
                "\nOpcion invalida.\n");

            break;
        }
    }
}

static void ejecutarGestionUsuarios(GestorUsuarios *gestor)
{
    int opcion = 0;

    while (
        opcion !=
        OPCION_USUARIO_VOLVER)
    {
        mostrarMenuUsuarios();

        opcion = leerEntero(
            "Seleccione una opcion: ");

        switch (opcion)
        {
        case OPCION_USUARIO_CREAR:
        {
            char *identificacion;
            char *nombre;
            char *direccion;

            int resultado;

            printf(
                "\nNumero de identificacion: ");

            identificacion =
                leerLineaDinamica();

            if (
                identificacion == NULL)
            {
                printf(
                    "Error al leer la identificacion.\n");

                break;
            }

            printf(
                "Nombre: ");

            nombre =
                leerLineaDinamica();

            if (
                nombre == NULL)
            {
                free(
                    identificacion);

                printf(
                    "Error al leer el nombre.\n");

                break;
            }

            printf(
                "Direccion: ");

            direccion =
                leerLineaDinamica();

            if (
                direccion == NULL)
            {
                free(
                    identificacion);

                free(
                    nombre);

                printf(
                    "Error al leer la direccion.\n");

                break;
            }

            resultado =
                crearUsuario(
                    gestor,
                    identificacion,
                    nombre,
                    direccion);

            if (resultado == 1)
            {
                guardarUsuariosJson(
                    gestor,
                    ARCHIVO_USUARIOS);

                printf(
                    "\nUsuario creado correctamente.\n");
            }
            else if (resultado == -1)
            {
                printf(
                    "\nYa existe un usuario con esa identificacion.\n");
            }
            else
            {
                printf(
                    "\nNo fue posible crear el usuario.\n");
            }

            free(
                identificacion);

            free(
                nombre);

            free(
                direccion);

            break;
        }

        case OPCION_USUARIO_VER:

            mostrarUsuarios(
                gestor);

            break;

        case OPCION_USUARIO_MODIFICAR:
        {
            char *identificacion;

            char *nombre;

            char *direccion;

            int resultado;

            printf(
                "\nIdentificacion del usuario: ");

            identificacion =
                leerLineaDinamica();

            printf(
                "Nuevo nombre: ");

            nombre =
                leerLineaDinamica();

            printf(
                "Nueva direccion: ");

            direccion =
                leerLineaDinamica();

            if (
                identificacion == NULL ||
                nombre == NULL ||
                direccion == NULL)
            {
                printf(
                    "\nError al leer los datos.\n");

                free(
                    identificacion);

                free(
                    nombre);

                free(
                    direccion);

                break;
            }

            resultado =
                modificarUsuario(
                    gestor,
                    identificacion,
                    nombre,
                    direccion);

            if (resultado == 1)
            {
                guardarUsuariosJson(
                    gestor,
                    ARCHIVO_USUARIOS);

                printf(
                    "\nUsuario modificado correctamente.\n");
            }
            else if (resultado == -1)
            {
                printf(
                    "\nUsuario no encontrado.\n");
            }
            else
            {
                printf(
                    "\nNo fue posible modificar el usuario.\n");
            }

            free(
                identificacion);

            free(
                nombre);

            free(
                direccion);

            break;
        }

        case OPCION_USUARIO_ELIMINAR:
        {
            char *identificacion;

            int resultado;

            printf(
                "\nIdentificacion del usuario por eliminar: ");

            identificacion =
                leerLineaDinamica();

            if (
                identificacion == NULL)
            {
                printf(
                    "Error al leer la identificacion.\n");

                break;
            }

            resultado =
                eliminarUsuario(
                    gestor,
                    identificacion);

            if (resultado == 1)
            {
                guardarUsuariosJson(
                    gestor,
                    ARCHIVO_USUARIOS);

                printf(
                    "\nUsuario eliminado correctamente.\n");
            }
            else if (resultado == -1)
            {
                printf(
                    "\nUsuario no encontrado.\n");
            }
            else if (resultado == -2)
            {
                printf(
                    "\nNo se puede eliminar el usuario.\n");

                printf(
                    "El usuario posee registros asociados.\n");
            }
            else
            {
                printf(
                    "\nNo fue posible eliminar el usuario.\n");
            }

            free(
                identificacion);

            break;
        }

        case OPCION_USUARIO_VOLVER:

            printf(
                "\nVolviendo a Opciones Operativas...\n");

            break;

        default:

            printf(
                "\nOpcion invalida.\n");

            break;
        }
    }
}

void destruirSistema(Sistema *sistema)
{
    if (sistema == NULL)
    {
        return;
    }

    destruirCatalogo(
        sistema->catalogo);

    destruirGestorUsuarios(
        sistema->gestorUsuarios);

    free(
        sistema);
}