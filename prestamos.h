#ifndef PRESTAMOS_H
#define PRESTAMOS_H

#include "catalogo.h"
#include "usuarios.h"

/* Tipo opaco: los detalles internos se mantienen en prestamos.c. */
typedef struct GestorPrestamos GestorPrestamos;

GestorPrestamos *crearGestorPrestamos(void);

int cargarPrestamosDesdeJson(
    GestorPrestamos *gestor,
    const char *rutaArchivo);

int guardarPrestamosJson(
    const GestorPrestamos *gestor,
    const char *rutaArchivo);

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
    const char *rutaUsuarios);

int registrarDevolucion(
    GestorPrestamos *gestor,
    const char *identificadorPrestamo,
    const char *fechaDevolucion,
    const char *rutaPrestamos);

int ejemplarEstaDisponible(
    const GestorPrestamos *gestor,
    const char *identificadorEjemplar);

void destruirGestorPrestamos(
    GestorPrestamos *gestor);

#endif
