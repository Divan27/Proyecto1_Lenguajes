#include <stdio.h>
#include <stdlib.h>

#include "sistema.h"
#include "menu.h"
#include "entrada.h"
#include "configuracion.h"
#include "catalogo.h"
#include "usuarios.h"
#include "prestamos.h"

struct Sistema
{
    int activo;

    Catalogo *catalogo;

    GestorUsuarios *gestorUsuarios;

    GestorPrestamos *gestorPrestamos;
};

static void ejecutarMenuPrincipal(Sistema *sistema);

static void ejecutarMenuOperativo(
    Sistema *sistema);

static void ejecutarGestionCatalogo(
    Catalogo *catalogo);

static void ejecutarMenuGeneral(
    Sistema *sistema);

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

    /* ========================= */
    /* CREAR GESTOR PRESTAMOS    */
    /* ========================= */

    /* Se crea el gestor que administrara prestamos y devoluciones. */
    nuevoSistema->gestorPrestamos =
        crearGestorPrestamos();

    if (nuevoSistema->gestorPrestamos == NULL)
    {
        printf(
            "Error creando el gestor de prestamos.\n");

        destruirGestorUsuarios(
            nuevoSistema->gestorUsuarios);

        destruirCatalogo(
            nuevoSistema->catalogo);

        free(nuevoSistema);
        return NULL;
    }

    if (
        !cargarPrestamosDesdeJson(
            nuevoSistema->gestorPrestamos,
            ARCHIVO_PRESTAMOS))
    {
        printf(
            "Advertencia: no fue posible cargar prestamos.json\n");
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

            ejecutarMenuGeneral(sistema);

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

static void ejecutarMenuGeneral(
    Sistema *sistema)
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
        {
            /* Datos necesarios para registrar un nuevo prestamo. */
            char *usuario;
            char *fechaInicio;
            char *fechaEntrega;
            char **identificadores;
            int cantidad;
            int i;

            printf("\nIdentificacion del usuario: ");
            usuario = leerLineaDinamica();

            printf("Fecha de inicio (AAAA-MM-DD): ");
            fechaInicio = leerLineaDinamica();

            printf("Fecha de entrega (AAAA-MM-DD): ");
            fechaEntrega = leerLineaDinamica();

            if (
                usuario == NULL ||
                fechaInicio == NULL ||
                fechaEntrega == NULL)
            {
                printf("\nNo fue posible leer los datos del prestamo.\n");
                free(usuario);
                free(fechaInicio);
                free(fechaEntrega);
                break;
            }

            cantidad = leerEntero(
                "Cantidad de ejemplares que desea solicitar: ");

            if (cantidad <= 0)
            {
                printf("\nLa cantidad debe ser mayor que cero.\n");
                free(usuario);
                free(fechaInicio);
                free(fechaEntrega);
                break;
            }

            /* El arreglo es dinamico porque la cantidad la decide el usuario. */
            identificadores = malloc(
                cantidad * sizeof(char *));

            if (identificadores == NULL)
            {
                printf("\nNo fue posible reservar memoria.\n");
                free(usuario);
                free(fechaInicio);
                free(fechaEntrega);
                break;
            }

            for (i = 0; i < cantidad; i++)
            {
                printf(
                    "Identificador del ejemplar %d: ",
                    i + 1);

                identificadores[i] =
                    leerLineaDinamica();

                if (identificadores[i] == NULL)
                {
                    int j;

                    printf("\nNo fue posible leer el identificador.\n");

                    for (j = 0; j < i; j++)
                    {
                        free(identificadores[j]);
                    }

                    free(identificadores);
                    identificadores = NULL;
                    break;
                }
            }

            if (identificadores != NULL)
            {
                /* La logica y validaciones del prestamo se manejan en prestamos.c. */
                registrarPrestamo(
                    sistema->gestorPrestamos,
                    sistema->catalogo,
                    sistema->gestorUsuarios,
                    usuario,
                    fechaInicio,
                    fechaEntrega,
                    identificadores,
                    cantidad,
                    ARCHIVO_PRESTAMOS,
                    ARCHIVO_USUARIOS);

                for (i = 0; i < cantidad; i++)
                {
                    free(identificadores[i]);
                }

                free(identificadores);
            }

            free(usuario);
            free(fechaInicio);
            free(fechaEntrega);

            break;
        }

        case 4:
        {
            /* Para devolver solo se necesita localizar el prestamo y su fecha real. */
            char *identificadorPrestamo;
            char *fechaDevolucion;

            printf("\nIdentificador del prestamo: ");
            identificadorPrestamo = leerLineaDinamica();

            printf("Fecha de devolucion (AAAA-MM-DD): ");
            fechaDevolucion = leerLineaDinamica();

            if (
                identificadorPrestamo == NULL ||
                fechaDevolucion == NULL)
            {
                printf("\nNo fue posible leer los datos de devolucion.\n");
                free(identificadorPrestamo);
                free(fechaDevolucion);
                break;
            }

            /* Calcula el monto y cambia el estado del prestamo a FINALIZADO. */
            registrarDevolucion(
                sistema->gestorPrestamos,
                identificadorPrestamo,
                fechaDevolucion,
                ARCHIVO_PRESTAMOS);

            free(identificadorPrestamo);
            free(fechaDevolucion);

            break;
        }

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

    destruirGestorPrestamos(
        sistema->gestorPrestamos);

    free(
        sistema);
}