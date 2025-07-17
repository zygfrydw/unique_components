import json, re
import pandas as pd
import matplotlib.pyplot as plt

with open("results.json") as f:
    data = json.load(f)

df = pd.DataFrame(data["benchmarks"])

# Split "name" into algo and args: e.g., "BM_A/1024" -> algo="BM_A", args="1024"
algo = df["name"].str.split("/", n=1, expand=True)
df["algo"] = algo[0]
df["arg_str"] = algo[1]

# Extract the first integer from arg_str as N (adjust regex if you have kB suffixes)
df["N"] = df["arg_str"].str.extract(r"(\d+)").astype(float)

# Convert times: google-benchmark reports ns by default in JSON unless you changed units.
# real_time & cpu_time are already numeric; divide to get microseconds, milliseconds, etc. if desired.
df["real_ms"] = df["real_time"] / 1e6

for name, g in df.groupby("algo"):
    g = g.sort_values("N")
    plt.plot(g["N"], g["real_ms"], marker="o", label=name)

plt.xlabel("N")
plt.ylabel("Real time (ms)")
plt.title("Benchmark results")
plt.legend()
plt.tight_layout()
plt.savefig('results.png')
