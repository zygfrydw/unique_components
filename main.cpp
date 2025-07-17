// island_algorithms_bench.cpp
// Compare DFS flood‑fill vs. single‑pass CCL using Google Benchmark
//---------------------------------------------------------------
#include <benchmark/benchmark.h>
#include <vector>
#include <stack>
#include <unordered_set>
#include <random>
#include <numeric>
#include <iostream>
#include <span>

class Map{
public:
    Map(int rows, int cols) : height(rows), width(cols) {
        data = new bool[rows * cols];
        std::fill(data, data + rows * cols, false);
    }
    ~Map() {
        delete[] data;
    }

    bool operator()(int r, int c) const {
        if (r < 0 || r >= height || c < 0 || c >= width) return false;
        return data[r * width + c];
    }

    bool& operator()(int r, int c) {
        return data[r * width + c];
    }

    int height, width;
    bool *data;
};


static void generateRandomGrid(Map& map, double landProb = 0.45)
{
    std::mt19937 rng(12345u);              // fixed seed for repeatability
    std::bernoulli_distribution bern(landProb);

    for (int r = 0; r < map.height; ++r)
        for (int c = 0; c < map.width; ++c)
            map(r, c) = bern(rng);
}

int countComponentsDFS(const Map& map)
{
    struct Point {
        int x, y;
    };
    const int rows = map.height;
    const int cols = map.width;

    Map visited(rows, cols);
    auto inside = [&](int r, int c){ return r>=0 && r<rows && c>=0 && c<cols; };

    int islands = 0;
    std::stack<Point> st;

    for (int r=0; r<rows; ++r)
        for (int c=0; c<cols; ++c)
            if (map(r, c) && !visited(r, c))
            {
                ++islands;
                st.push({c,r});
                while (!st.empty())
                {
                    auto [x,y] = st.top(); st.pop();
                    if (!inside(y,x) || visited(y,x) || !map(y, x)) continue;
                    visited(y,x) = 1;
                    st.push({x+1,y}); st.push({x-1,y});
                    st.push({x,y+1}); st.push({x,y-1});
                }
            }

    return islands;
}


int countComponentsSauf(const Map& img)
{
    const int  W = img.width;
    const int  H = img.height;
    const bool* px = img.data;

    if (W <= 0 || H <= 0 || px == nullptr) return 0;

    std::vector<int> prev_labels(W, 0);
    std::vector<int> curr_labels(W, 0);

    // Union–Find parent table; parent[0] is dummy (background).
    std::vector<int> parent(1, 0);

    auto find_root = [&](int x) {
        int r = x;
        while (parent[r] != r)           // climb to root
            r = parent[r];
        while (parent[x] != r) {         // path compression
            int p = parent[x];
            parent[x] = r;
            x = p;
        }
        return r;
    };

    auto unite = [&](int a, int b) {
        int ra = find_root(a);
        int rb = find_root(b);
        if (ra != rb) parent[rb] = ra;    // rb -> ra
    };

    int nextLabel = 1;

    // ---------- FIRST PASS ----------
    for (int y = 0; y < H; ++y) {
        const int rowOff = y * W;
        std::swap(prev_labels, curr_labels); // swap current and previous labels
        for (int x = 0; x < W; ++x) {
            if (!px[rowOff + x]){
                curr_labels[x] = 0;
                continue;
            }

            // neighbours: A = north, D = west
            const int N = (y > 0 ? prev_labels[x] : 0);
            const int W = (x > 0 ? curr_labels[x-1] : 0);

            // SAUF decision tree (4 cases)
            if (N == 0 && W == 0) {                   // new component
                curr_labels[x] = nextLabel;
                parent.push_back(nextLabel);          // parent[i] = i
                ++nextLabel;

            } else if (N != 0 && W == 0) {            // copy A
                curr_labels[x] = N;

            } else if (N == 0 && W != 0) {            // copy D
                curr_labels[x] = W;

            } else {                                  // A & D both > 0
                const int rA = find_root(N);
                const int rD = find_root(W);
                curr_labels[x] = (rA < rD ? rA : rD);
                if (rA != rD) unite(rA, rD);          // record equivalence
            }
        }
    }

    // ---------- SECOND PASS (count unique roots) ----------
    std::unordered_set<int> uniqueRoots;
    for (int lbl = 1; lbl < nextLabel; ++lbl)
        if (parent[lbl])                              // skip unused slots
            uniqueRoots.insert(find_root(lbl));

    return static_cast<int>(uniqueRoots.size());
}

// --------------------------------------------------------------
// Google Benchmark wrappers
// --------------------------------------------------------------
static void BM_DFS(benchmark::State& state)
{
    const int rows = static_cast<int>(state.range(0));
    const int cols = static_cast<int>(state.range(1));
    for (auto _ : state)
    {
        state.PauseTiming();
        Map grid(rows, cols);
        generateRandomGrid(grid, 0.45);
        state.ResumeTiming();
        benchmark::DoNotOptimize(countComponentsDFS(grid));
    }
}
BENCHMARK(BM_DFS)->Args({256,256})->Args({512,512})->Args({1024,1024})->Args({4096,4096})->Args({8192,8192})->Args({16384,16384});

static void BM_SAUF(benchmark::State& state)
{
    const int rows = static_cast<int>(state.range(0));
    const int cols = static_cast<int>(state.range(1));
    for (auto _ : state)
    {
        state.PauseTiming();
        Map grid(rows, cols);
        generateRandomGrid(grid, 0.45);
        state.ResumeTiming();
        benchmark::DoNotOptimize(countComponentsSauf(grid));
    }
}
BENCHMARK(BM_SAUF)->Args({256,256})->Args({512,512})->Args({1024,1024})->Args({4096,4096})->Args({8192,8192})->Args({16384,16384});

BENCHMARK_MAIN();

// int main(){
//     const int N = 512;
//     Map map(N, N);
//     generateRandomGrid(map, 0.45);
   
//     std::cout << "Number of islands (SAUF): " << countComponentsSauf(map) << '\n';
//     std::cout << "Number of islands (DFS): " << countComponentsDFS(map) << '\n';
//     return 0;
// }
