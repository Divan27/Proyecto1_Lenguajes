#ifndef HISTORIAL_H
#define HISTORIAL_H

void mostrarHistorialPrestamos(
    const char *rutaArchivo,
    const char *fechaInicio,
    const char *fechaFin);

int fechaValida(
    const char *fecha);

#endif