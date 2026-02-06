#include <iostream>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <iomanip>

struct ThroughputResult {
    double avg_packets_per_slot;
    double avg_percent;
};

class ApsaraSwitch {
public:
    explicit ApsaraSwitch(int n_ports, uint64_t seed = 0)
        : n(n_ports),
          Q(n_ports, std::vector<int>(n_ports, 0)),
          matching(n_ports),
          rng(seed),
          dist_out(0, n_ports - 1)
    {
        std::iota(matching.begin(), matching.end(), 0);
    }

    void update_buffers() {
        for (int i = 0; i < n; ++i) {
            int outp = dist_out(rng);
            Q[i][outp] += 1;
        }
    }

    int weight(const std::vector<int>& m) const {
        int w = 0;
        for (int i = 0; i < n; ++i) w += Q[i][m[i]];
        return w;
    }

    std::vector<int> hamiltonian_matching(int time_slot) const {
        int k = time_slot % n;
        std::vector<int> m(n);
        for (int i = 0; i < n; ++i) m[i] = (i + k) % n;
        return m;
    }

    void generate_neighbors(const std::vector<int>& cur,
                            std::vector<std::vector<int>>& neighbors) const {
        neighbors.clear();
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                std::vector<int> nb = cur;
                std::swap(nb[i], nb[j]);
                neighbors.push_back(std::move(nb));
            }
        }
    }

    const std::vector<int>& schedule(int time_slot) {
        std::vector<std::vector<int>> neighbors;
        generate_neighbors(matching, neighbors);

        std::vector<int> best = matching;
        int best_w = weight(best);

        for (const auto& nb : neighbors) {
            int w = weight(nb);
            if (w > best_w) {
                best_w = w;
                best = nb;
            }
        }

        auto ham = hamiltonian_matching(time_slot);
        if (weight(ham) > best_w) best = std::move(ham);

        matching = std::move(best);
        return matching;
    }

    int serve_and_get_sent(const std::vector<int>& m) {
        int sent = 0;
        for (int i = 0; i < n; ++i) {
            int outp = m[i];
            if (Q[i][outp] > 0) {
                Q[i][outp] -= 1;
                sent += 1;
            }
        }
        return sent;
    }

    ThroughputResult run(int time_slots, bool verbose = false) {
        long long total_sent = 0;

        for (int t = 1; t <= time_slots; ++t) {
            update_buffers();
            const auto& m = schedule(t);
            int sent = serve_and_get_sent(m);
            total_sent += sent;

            if (verbose && (t <= 20 || t % 10000 == 0)) {
                double percent = (double(sent) / n) * 100.0;
                std::cout << "Slot " << t
                          << " | sent=" << sent
                          << " | throughput=" << percent << "%\n";
            }
        }

        ThroughputResult res;
        res.avg_packets_per_slot =
            double(total_sent) / double(time_slots);
        res.avg_percent =
            (res.avg_packets_per_slot / double(n)) * 100.0;

        return res;
    }

private:
    int n;
    std::vector<std::vector<int>> Q;
    std::vector<int> matching;
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> dist_out;
};

void run_custom() {
    int n_ports, T;
    std::cout << "Enter number of ports: ";
    std::cin >> n_ports;
    std::cout << "Enter time slots: ";
    std::cin >> T;

    ApsaraSwitch sw(n_ports, 20260206ULL);
    auto res = sw.run(T, true);

    std::cout << "\nAverage throughput:\n";
    std::cout << std::fixed << std::setprecision(6)
              << res.avg_packets_per_slot << " packets/slot\n";
    std::cout << res.avg_percent << " %\n";
}

void run_ports_4_to_8_csv(int T) {
    std::ofstream fout("throughput_vs_ports.csv");
    fout << "ports,avg_packets_per_slot,avg_throughput_percent\n";

    for (int p = 4; p <= 8; ++p) {
        ApsaraSwitch sw(p, 20260206ULL + p);
        auto res = sw.run(T, false);

        fout << p << ","
             << std::fixed << std::setprecision(8)
             << res.avg_packets_per_slot << ","
             << res.avg_percent << "\n";

        std::cout << "ports=" << p
                  << " avg=" << res.avg_packets_per_slot
                  << " (" << res.avg_percent << "%)\n";
    }

    fout.close();
}

int main() {
    const int STUDENT_LAST6 = 131913;

    while (true) {
        std::cout << "\n--- APSARA Matching Scheduler ---\n";
        std::cout << "1) Run custom simulation\n";
        std::cout << "2) Ports 4..8 with T=131913 (CSV)\n";
        std::cout << "3) Exit\n";
        std::cout << "Choice: ";

        int c;
        if (!(std::cin >> c)) break;

        if (c == 1) run_custom();
        else if (c == 2) run_ports_4_to_8_csv(STUDENT_LAST6);
        else if (c == 3) break;
    }

    return 0;
}
