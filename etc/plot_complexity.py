import sys, math
import pandas as pd
import matplotlib.pyplot as plt

timing_csv = sys.argv[1]
fig, ax_list = plt.subplots(figsize=(20,6), ncols=3)

algo_to_ax = {
    'DAG' : ax_list[0],
    'DCYCLE' : ax_list[1],
    'UCYCLE' : ax_list[2],
}

def linear_model (start, slope):
  def __inner_model__(x):
    return start + slope * x.name
  return __inner_model__

def nlogn_model (start, log_slope):
  def __inner_model__(x):
    return start + log_slope * x.name * math.log(x.name or 0.0001)
  return __inner_model__

def build_group_df (df):
  df_group = df.reset_index(drop=True)
  x_len = df_group.size - 1
  start = df_group.total_time.loc[0]
  end = df_group.total_time.loc[-1]
  slope = (end - start) / x_len
  log_slope = (end - start) / (x_len * math.log(x_len))

  df_group['lin_model'] = df_group.apply(axis=1, func=linear_model(start, slope))
  df_group['log_model'] = df_group.apply(axis=1, func=nlogn_model(start, log_slope))
  return df_group

for name, ax in algo_to_ax.items():
    ax.set_title('Graph type : ' + name)

df = pd.read_csv(timing_csv, sep='\s*,\s*')
df = df.loc[:, ['name', 'total_time']]
dfg = df.groupby('name')

for name, group in dfg:
    algo = name.split('_')[-1]
    ax = algo_to_ax[algo]
    group.reset_index(drop=True).plot(ax=ax, y='total_time', label=name)

plt.show()

