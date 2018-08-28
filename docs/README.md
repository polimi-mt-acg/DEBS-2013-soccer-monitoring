
# Speedup Analysis

This notebook is about **exploring** the effects on the application **performance** of the following parameters:
 - **Number of threads**: the parallelization factor
 - **Batch size**: the number of events to gather before processing them in parallel

### Application interface
The `soccer-monitoring` application exposes the following interface through the command-line

```bash
$ soccer-monitoring -h                                                                                                        
DEBS 2013 - Soccer Monitoring tool:
  -h [ --help ]                   Print this message
  -T [ --time-units ] arg         Frequency of statistics (in seconds)
  -K [ --max-distance ] arg       Maximum distance for ball possession 
                                  eligibility
  -s [ --stream ] arg             Game stream file path
  -m [ --metadata ] arg           Metadata file path
  -t [ --threads ] arg (=0)       Number of threads
  -B [ --batch-size ] arg (=1500) Events batch size (default: auto)
  -o [ --output ] arg             Output file path (default: stdout)
```

### Design of experiment
In order to explore the domain space, we are going to measure the time required _on average_ to process a batch of events, subject to the parameters changing. Concerning the **number of threads** we are testing the values $nb\_threads = \left \{1, 2, 4, 8 \right \}$, while for **batch size** we are testing the values $batch\_size = \left \{1, 10, 10^2, 10^3, 10^4, 10^5, 10^6, 10^7 \right \}$.

As for the other parameters, we keep them fixed
 - Frequency of statistics = $60 s$
 - Maximum distance for ball possession eligibility = $1 m$
 
### Equipment
Macbook Pro Mid-2012 shipping a Intel(R) Core(TM) i7-3615QM CPU @ 2.30GHz.


```python
import numpy as np
import pandas as pd
import re
import subprocess
import matplotlib.pyplot as plt
from pathlib import Path

%matplotlib inline

# Set Notebook running mode:
is_demo = True
```


```python
args = ['../cmake-build-release/soccer-monitoring', \
        '-T', '60', \
        '-K', '1', \
        '-s', '../test/resources/game_data_start_10_1e7', \
        '-m', '../datasets/preprocessed/metadata']
nb_threads = [1, 2, 4, 8]
batch_size = [int(10 ** x) for x in range(0, 8)]
pattern = r'Processed \d+ seconds of the stream \(~ \d+ events\) in (\d+\.\d+) seconds'
pattern = re.compile(pattern)
```

## Full space exploration


```python
op_path = Path('./operating_points.npy')
if is_demo and op_path.is_file():
    data = np.load(str(op_path))
else:
    data = np.zeros((len(nb_threads), len(batch_size), 12))
    for i, thread in enumerate(nb_threads):
        for j, size in enumerate(batch_size):
            out = subprocess.run(args=args + ['-t', '{}'.format(thread), '-B', '{}'.format(size)], \
                                 check=True, \
                                 capture_output=True, \
                                 text=True)
            ts = [float(m.group(1)) for m in re.finditer(pattern, out.stdout)]
            data[i, j] = ts
```


```python
def make_dse_df(data):
    sorted_data = np.sort(data, axis=2)
    batch_time = np.mean(sorted_data[::, ::, 1:-1], axis=2)
    columns = ['1e+{}'.format(x) for x in range(0, 8)]
    index = [str(t) for t in nb_threads]

    df = pd.DataFrame(data=batch_time, columns=columns, index=index)
    speedup = (df['1e+0'].T / df.T).T - 1
    speedup['1e+0'] += 2e-2
    speedup = speedup.rename(columns={'BatchTime': 'Speedup'})
    return df, speedup

def make_scatter_df(df, value_col_name):
    scatter_data = []
    for j, col in enumerate(df.columns.tolist()):
        for i, index in enumerate(df.index.tolist()):
            scatter_data += [(2 ** i, 10 ** j, df.at[index, col])]
    columns = ['NbThreads', 'BatchSize', value_col_name]
    return pd.DataFrame(data=scatter_data, columns=columns)

exec_time, speedup = make_dse_df(data)
exec_time_scatter = make_scatter_df(exec_time, 'BatchTime')
speedup_scatter = make_scatter_df(speedup, 'SpeedUp')

plt.figure(figsize=(15, 8))
plt.clf()

ax = plt.subplot(211)
ax.set_yscale('log', basey=2)
exec_time_scatter.plot.scatter(x='BatchSize', y='NbThreads', c='BatchTime', s=200, logx=True, colormap='viridis', ax=ax)

ax = plt.subplot(212)
ax.set_yscale('log', basey=2)
speedup_scatter.plot.scatter(x='BatchSize', y='NbThreads', c='SpeedUp', s=200, logx=True, colormap='viridis', ax=ax)

idx_max = speedup_scatter.idxmax()
max_point = speedup_scatter.iloc[idx_max['SpeedUp']]
max_nb_threads = int(max_point['NbThreads'])
max_batch_size = int(max_point['BatchSize'])
print('Max speedup: {:1.3f}%'.format(max_point['SpeedUp']))
print('  NbThreads: {:d}'.format(max_nb_threads))
print('  BatchSize: {:d}'.format(max_batch_size))
```

    Max speedup: 4.179%
      NbThreads: 8
      BatchSize: 1000000



![png](speedup_analysis_files/speedup_analysis_7_1.png)



```python
# Store computed operating points
if not op_path.is_file():
    np.save('./operating_points.npy', data)
```

## Number of threads analysis


```python
idx = max_nb_threads
exec_time_t = exec_time.loc[str(idx)]
speedup_t = speedup.loc[str(idx)]

plt.figure(figsize=(15, 8))
plt.clf()

ax = plt.subplot(211)
exec_time_t.plot.bar(y='BatchTime', legend=False, title='Batch time (seconds) [NbThreads = {}]'.format(max_nb_threads), rot=0, ax=ax)

ax = plt.subplot(212)
speedup_t.plot.bar(y='Speedup', legend=False, title='Speedup (%) [NbThreads = {}]'.format(max_nb_threads), rot=0, ax=ax)
```




    <matplotlib.axes._subplots.AxesSubplot at 0x7f9e47c90160>




![png](speedup_analysis_files/speedup_analysis_10_1.png)


## Batch size analysis


```python
col = '1e+{}'.format(int(np.log10(max_batch_size)))
exec_time_b = exec_time[col]
speedup_b = speedup[col]

plt.figure(figsize=(15, 8))
plt.clf()

ax = plt.subplot(211)
exec_time_b.plot.bar(y='BatchTime', legend=False, title='Batch time (seconds) [BatchSize = {}]'.format(col), rot=0, ax=ax)

ax = plt.subplot(212)
speedup_b.plot.bar(y='Speedup', legend=False, title='Speedup (%) [BatchSize = {}]'.format(col), rot=0, ax=ax)
```




    <matplotlib.axes._subplots.AxesSubplot at 0x7f9e47970940>




![png](speedup_analysis_files/speedup_analysis_12_1.png)

