import pandas as pd
import matplotlib.pyplot as plt

# Leer CSV con tiempos paralelos
df = pd.read_csv("tiempos_paralelos.csv")

# Tiempos secuenciales por tamaño de problema
tiempos_secuenciales = {
    1000: 0.021238,
    10000: 0.062260,
    100000: 0.529213,
    1000000: 5.312929,
    10000000: 53.353924,
    100000000: 533.940735,
}

# Crear columna de tiempos secuenciales en el DataFrame
df["Tiempo_Secuencial"] = df["Tamaño"].map(tiempos_secuenciales)

# Calcular speedup y eficiencia
df["Speedup"] = df["Tiempo_Secuencial"] / df["Tiempo"]
df["Eficiencia"] = df["Speedup"] / df["Nodos"]

# Guardar nuevo CSV con métricas
df.to_csv("tiempos_paralelos_con_metricas.csv", index=False)

# Graficar speedup y eficiencia por tamaño (individuales)
for size in df["Tamaño"].unique():
    subdf = df[df["Tamaño"] == size]
    nodos = subdf["Nodos"]
    
    plt.figure(figsize=(12, 4))
    
    # Gráfico de Speedup
    plt.subplot(1, 2, 1)
    plt.plot(nodos, subdf["Speedup"], marker='o', label="Speedup")
    plt.plot(nodos, nodos, linestyle='--', color='gray', label="Speedup ideal")
    plt.xticks(nodos)
    plt.title(f"Speedup - Tamaño {size}")
    plt.xlabel("Nodos")
    plt.ylabel("Speedup")
    plt.grid(True)
    plt.legend()
    
    # Gráfico de Eficiencia
    plt.subplot(1, 2, 2)
    plt.plot(nodos, subdf["Eficiencia"], marker='o', color='orange', label="Eficiencia")
    plt.axhline(y=1.0, color='gray', linestyle='--', label="Eficiencia ideal")
    plt.xticks(nodos)
    plt.title(f"Eficiencia - Tamaño {size}")
    plt.xlabel("Nodos")
    plt.ylabel("Eficiencia")
    plt.grid(True)
    plt.legend()
    
    plt.tight_layout()
    plt.savefig(f"grafico_tamano_{size}.png")
    plt.close()

# Gráfico combinado: SPEEDUP
plt.figure(figsize=(10, 6))
for size in sorted(df["Tamaño"].unique()):
    subdf = df[df["Tamaño"] == size]
    plt.plot(subdf["Nodos"], subdf["Speedup"], marker='o', label=f"Tamaño {size}")
plt.plot(subdf["Nodos"], subdf["Nodos"], linestyle='--', color='gray', label="Speedup ideal")
plt.xlabel("Nodos")
plt.ylabel("Speedup")
plt.title("Speedup combinado por tamaño")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("speedup_combinado.png")
plt.close()

# Gráfico combinado: EFICIENCIA
plt.figure(figsize=(10, 6))
for size in sorted(df["Tamaño"].unique()):
    subdf = df[df["Tamaño"] == size]
    plt.plot(subdf["Nodos"], subdf["Eficiencia"], marker='o', label=f"Tamaño {size}")
plt.axhline(y=1.0, linestyle='--', color='gray', label="Eficiencia ideal")
plt.xlabel("Nodos")
plt.ylabel("Eficiencia")
plt.title("Eficiencia combinada por tamaño")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("eficiencia_combinada.png")
plt.close()
