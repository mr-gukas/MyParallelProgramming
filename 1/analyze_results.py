import pandas as pd
import matplotlib.pyplot as plt
import os

def main():
    if not os.path.exists('plots'):
        os.makedirs('plots')

    df = pd.read_csv('results.csv')
    seq_df = df[(df['mode']=='seq') & (df['P']==1)]

    mpi_df = df[df['mode']=='mpi']
    for (M, K), group in mpi_df.groupby(['M','K']):
        grp = group.sort_values('P').copy()
        T1 = float(seq_df[(seq_df['M']==M) & (seq_df['K']==K)]['time'].values[0])
        grp['speedup']    = T1 / grp['time']
        grp['efficiency'] = grp['speedup'] / grp['P']

        plt.figure()
        plt.plot(grp.P, grp.speedup, '-o', label=f'M={M},K={K}')
        plt.xlabel('Число процессов P')
        plt.ylabel('Ускорение S(P)')
        plt.title(f'Strong scaling: M={M}, K={K}')
        plt.grid(True)
        plt.savefig(f'plots/speedup_M{M}_K{K}.png')
        plt.close()

        plt.figure()
        plt.plot(grp.P, grp.efficiency, '-o', label=f'M={M},K={K}')
        plt.xlabel('Число процессов P')
        plt.ylabel('Эффективность E(P)')
        plt.title(f'Efficiency: M={M}, K={K}')
        plt.grid(True)
        plt.savefig(f'plots/efficiency_M{M}_K{K}.png')
        plt.close()

    print("Plots saved in current directory.")

if __name__ == '__main__':
    main() 