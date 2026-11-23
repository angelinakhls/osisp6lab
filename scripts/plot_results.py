import sys
import os
import csv
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def load_benchmark_csv(csv_path):
    grouped = defaultdict(lambda: {"sum": [], "min": [], "max": [], "avg": [], "total": []})
    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            n = int(row["size"])
            grouped[n]["sum"].append(float(row["sum_ms"]))
            grouped[n]["min"].append(float(row["min_ms"]))
            grouped[n]["max"].append(float(row["max_ms"]))
            grouped[n]["avg"].append(float(row["avg_ms"]))
            grouped[n]["total"].append(float(row["total_ms"]))
    return grouped


def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "../data/benchmark_results.csv"
    out_dir = sys.argv[2] if len(sys.argv) > 2 else "../diagrams"
    os.makedirs(out_dir, exist_ok=True)

    data = load_benchmark_csv(csv_path)
    sizes = sorted(data.keys())

    mean_total = [np.mean(data[n]["total"]) for n in sizes]
    mean_sum = [np.mean(data[n]["sum"]) for n in sizes]
    mean_min = [np.mean(data[n]["min"]) for n in sizes]
    mean_max = [np.mean(data[n]["max"]) for n in sizes]
    mean_avg = [np.mean(data[n]["avg"]) for n in sizes]

    plt.rcParams["font.family"] = "DejaVu Sans"

    # Chart 1: Total execution time vs array size
    fig, ax = plt.subplots(figsize=(7.5, 4.5), dpi=300)
    ax.plot(sizes, mean_total, marker="o", linewidth=2.2, color="#1f4e79", label="Суммарное время (4 функции)")
    for x, y in zip(sizes, mean_total):
        ax.annotate(f"{y:.2f} мс", (x, y), textcoords="offset points", xytext=(0, 8), ha="center", fontsize=9)
    ax.set_title("Зависимость времени выполнения функций DLL от размера массива\n(1000 итераций, среднее по 3 запускам)", fontsize=11, pad=12)
    ax.set_xlabel("Количество элементов массива, шт.", fontsize=10)
    ax.set_ylabel("Время выполнения, мс", fontsize=10)
    ax.set_xscale("log")
    ax.set_xticks(sizes)
    ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    ax.grid(True, linestyle="--", alpha=0.6)
    ax.legend(fontsize=9)
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "chart_execution_time.png"))
    plt.close(fig)

    # Chart 2: Per-function comparison across array sizes
    fig, ax = plt.subplots(figsize=(7.5, 4.5), dpi=300)
    x = np.arange(len(sizes))
    w = 0.2
    ax.bar(x - 1.5 * w, mean_sum, w, label="ArraySum", color="#2e75b6")
    ax.bar(x - 0.5 * w, mean_min, w, label="ArrayMin", color="#548235")
    ax.bar(x + 0.5 * w, mean_max, w, label="ArrayMax", color="#bf9000")
    ax.bar(x + 1.5 * w, mean_avg, w, label="ArrayAverage", color="#c55a11")
    ax.set_title("Сравнение времени выполнения экспортируемых функций DLL\n(1000 итераций, логарифмическая шкала)", fontsize=11, pad=12)
    ax.set_xlabel("Размер массива, элементов", fontsize=10)
    ax.set_ylabel("Среднее время выполнения, мс", fontsize=10)
    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels([str(s) for s in sizes])
    ax.grid(True, axis="y", linestyle="--", alpha=0.6)
    ax.legend(fontsize=9, ncol=2)
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, "chart_functions_breakdown.png"))
    plt.close(fig)

    print("Charts saved to:", out_dir)


if __name__ == "__main__":
    main()
