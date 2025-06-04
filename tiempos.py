import matplotlib.pyplot as plt

# Datos: Tamaño -> lista de (nodos, tiempo en segundos)
datos = {
    1000: [
        (2, 0.002901), (4, 0.003073), (8, 0.025749), (16, 0.089495), (32, 0.201565)
    ],
    10000: [
        (2, 0.026818), (4, 0.014584), (8, 0.030036), (16, 0.074260), (32, 0.182733)
    ],
    100000: [
        (2, 0.265535), (4, 0.139449), (8, 0.130072), (16, 0.126373), (32, 0.218815)
    ],
    1000000: [
        (2, 2.685180), (4, 1.359618), (8, 0.784534), (16, 0.543530), (32, 0.424249)
    ],
    10000000: [
        (2, 26.734681), (4, 13.474908), (8, 6.881816), (16, 3.915107), (32, 2.571745)
    ],
    100000000: [
        (2, 266.541988), (4, 134.135886), (8, 68.085407), (16, 40.945659), (32, 22.374585)
    ]
}

# Crear un gráfico por tamaño
for tamano, valores in datos.items():
    nodos = [n for n, _ in valores]
    tiempos_min = [t / 60 for _, t in valores]  # convertir a minutos

    plt.figure(figsize=(8, 5))
    plt.plot(nodos, tiempos_min, marker='o', linestyle='-', color='blue')
    plt.title(f"Tiempo de Ejecución promedio - Tamaño {tamano}")
    plt.xlabel("Cantidad de Nodos")
    plt.ylabel("Tiempo (minutos)")
    plt.grid(True)
    plt.xticks(nodos)  # mostrar solo los nodos usados
    plt.tight_layout()
    
    # Guardar el gráfico como imagen PNG
    plt.savefig(f"tiempo_tamano_{tamano}.png")
    plt.close()
