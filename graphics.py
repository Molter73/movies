import pandas as pd 
import matplotlib.pyplot as plt 
import os
import re
import sys
import numpy as np

def graphics(directorio):
    resultados = []

    for archivo in os.listdir(directorio):
        if re.match(r'^threads_\d+_\d+_\d+_\d+\.csv$', archivo):
            ruta_completa = os.path.join(directorio, archivo)
            df = pd.read_csv(ruta_completa)

            # Dividimos el archivo y extraemos Hilos y Método
            partes = archivo.split('_')
            num_hilos = int(partes[1])
            metodo = partes[4].split('.')[0]

            # Calculamos duración media
            duracion_media = df['Duracion'].mean()
            
            # Guardamos resultados
            resultados.append((num_hilos, metodo, duracion_media))
            print(f"Archivo leído: {archivo} - Método: {metodo} - Duración Media: {duracion_media}")

    # Creamos DataFrame y ordenamos
    resultados_df = pd.DataFrame(resultados, columns = ['Hilos', 'Método', 'Duración Media'])
    resultados_df = resultados_df.sort_values(by=['Hilos', 'Método'])

    # Graficamos
    plt.figure()
    max_hilos = resultados_df['Hilos'].max()
    min_hilos = resultados_df['Hilos'].min()
    max_duracion = resultados_df['Duración Media'].max()
    min_duracion = resultados_df['Duración Media'].min()

    for metodo in resultados_df['Método'].unique():
        df_metodo = resultados_df[resultados_df['Método'] == metodo]
        plt.plot(df_metodo['Hilos'], df_metodo['Duración Media'], label = f'Método {metodo}', marker='o')

    plt.title('Comparación entre los Métodos Utilizados')
    plt.xlabel('Número de Hilos')
    plt.ylabel('Duración Media (ns)')
    plt.legend()

    num_ticks = 10  # Número de ticks
    plt.xticks(ticks=np.linspace(min_hilos, max_hilos, num_ticks, dtype=int))
    plt.yticks(ticks=np.linspace(min_duracion, max_duracion, num_ticks))

    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 graphics.py <directorio>")
        sys.exit(1)

    directorio = sys.argv[1]
    graphics(directorio)