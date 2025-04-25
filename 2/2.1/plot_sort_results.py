#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import os  

def main():
    if not os.path.exists('plots'):
        os.makedirs('plots')
    df = pd.read_csv('sort_results.csv')
    seq_df = df[(df['mode']=='seq') & (df['P']==1)].set_index('N')

    par_df = df[df['mode']=='par']

    for N, group in par_df.groupby('N'):
        grp = group.sort_values('P').copy()
        T1 = float(seq_df.loc[N,'time'])
        grp['speedup']    = T1 / grp['time']
        grp['efficiency'] = grp['speedup'] / grp['P']

        # График ускорения (включая P=1)
        plt.figure()
        Ps = [1] + grp['P'].tolist()
        Su = [1.0] + grp['speedup'].tolist()
        plt.plot(Ps, Su, '-o')
        plt.title(f'Speedup (N={N})')
        plt.xlabel('Число потоков P')
        plt.ylabel('Ускорение S(P)')
        plt.grid(True)
        plt.savefig(f'plots/speedup_N{N}.png')
        plt.close()

        # График эффективности (включая P=1)
        plt.figure()
        Ps = [1] + grp['P'].tolist()
        Ef = [1.0] + grp['efficiency'].tolist()
        plt.plot(Ps, Ef, '-o')
        plt.title(f'Efficiency (N={N})')
        plt.xlabel('Число потоков P')
        plt.ylabel('Эффективность E(P)')
        plt.grid(True)
        plt.savefig(f'plots/efficiency_N{N}.png')
        plt.close()

    print('Plots created: speedup_N*.png and efficiency_N*.png')

if __name__ == '__main__':
    main() 