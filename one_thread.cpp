#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#define ITERATION_NUMBER 1000
#define START_TEMPERATURE 1000
#define TEMPERATURE_ITERATIONS 10
#define CR_U 0.4
#define BF_U 0.1

enum {
    BOLTZMANN,
    CAUCHY,
    LOGN_DIV_N,
};

enum {
    CR,
    BF,
    NO_CRITERIA,
};

enum {
    CSV,
    NO_OUTPUT,
};

int next_temperature_type = CAUCHY;
int additional_criteria = CR;
int OUT = NO_OUTPUT;

unsigned long long skipped_iterations = 0;

void print(std::vector<std::vector<int>> &a)
{
    for (int i = 0; i < a.size(); ++i) {
        for (int j = 0; j < a[i].size(); ++j) {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void dfs(int curr, std::vector<int> &res, std::vector<std::vector<int>> &graph, std::vector<bool> &used)
{
    used[curr] = true;
    for (int j = 0; j < graph[curr].size(); ++j) {
        if (!used[graph[curr][j]]) {
            dfs(graph[curr][j], res, graph, used);
        }
    }
    res.push_back(curr);
}

std::vector<int> topology_sort(std::vector<std::vector<int>> &graph)
{
    std::vector<int> res;
    std::vector<bool> used(graph.size(), false);
    for (int i = 0; i < graph.size(); ++i) {
        if (!used[i]) {
            dfs(i, res, graph, used);
        }
    }
    
    std::reverse(res.begin(), res.end());
    return res;
}

// returns number of predecessors on curr processor and new processor 
std::pair<int, int> check_CR_change_by_switch_processor(int curr_proc, int new_proc, int curr_tier, std::vector<int> &tasks, std::vector<int> &procs, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents)
{
    int on_curr_proc = 0, on_new_proc = 0;
    int curr_task = tasks[curr_tier];
    for (auto &task : graph[curr_task]) {
        for (int tier = 0; tier < tasks.size(); ++tier) {
            if (tasks[tier] == task) {
                if (procs[tier] == curr_proc) {
                    on_curr_proc++;
                } else if (procs[tier] == new_proc) {
                    on_new_proc++;
                }
                break;
            }
        }
    }
    for (auto &task : parents[curr_task]) {
        for (int tier = 0; tier < tasks.size(); ++tier) {
            if (tasks[tier] == task) {
                if (procs[tier] == curr_proc) {
                    on_curr_proc++;
                } else if (procs[tier] == new_proc) {
                    on_new_proc++;
                }
                break;
            }
        }
    }
    return std::pair<int, int>{on_curr_proc, on_new_proc};
}

class TierSchedule
{
public:
    std::vector<int> task;
    std::vector<int> proc;
    std::vector<int> proc_load; // for BF_criteria
    int transmitions = 0; // for CR criteria - number of transmitions in this schedule
    int all_transmitions = 0; // for CR criteria - number of transmitions in graph
    TierSchedule(int task_num, int proc_num, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents) {
        proc = std::vector<int>(task_num);
        proc_load = std::vector<int>(proc_num, 0);
        task = topology_sort(graph);
        // put tasks in random processors
        for (int i = 0; i < task_num; ++i) {
            // TODO describe generation BF schedule in course work
            switch (additional_criteria) {
                case BF:
                    { // block because of initialization of processors array
                        std::vector<int> processors(proc_num);
                        for (int j = 0; j < proc_num; ++j) {
                            processors[j] = j;
                        }
                        std::random_shuffle(processors.begin(), processors.end());
                        for (int j = 0; j < proc_num + 1; ++j) {
                            if (j == proc_num) {
                                std::cout << "Can't generate correct schedule for this BF criteria\n";
                                throw;
                            }
                            proc[i] = processors[j];
                            if (double((proc_load[proc[i]] + 1) * proc_num) / task_num - 1 < BF_U) {
                                break;
                            }
                        }
                    }
                    proc_load[proc[i]]++;
                    break;
                case CR:
                    // TODO describe generation CR schedule in course work
                    // Counts number of all transmitions and put all tasks on processor 0
                    // After this cycle try to move tasks to different processors
                    for (auto &el1 : graph[i]) {
                        all_transmitions++;
                    }
                    proc[i] = 0;
                    break;
                default:            
                    proc[i] = rand() % proc_num;
            }
        }
        switch (additional_criteria) {
        case CR:
            // try to move tasks to different processors with saving criteria bound
            {
                std::vector<int> rand_tiers(task_num);
                for (int i = 0 ; i < task_num; ++i) {
                    rand_tiers[i] = i;
                }
                std::random_shuffle(rand_tiers.begin(), rand_tiers.end());
                for (int i = 0; i < task_num; ++i) {
                    int curr_tier = rand_tiers[i];
                    int curr_proc = proc[curr_tier];
                    int new_proc =  rand() % proc_num;
                    std::pair<int, int> changes = check_CR_change_by_switch_processor(curr_proc, new_proc, curr_tier, task, proc, graph, parents);
                    int on_curr_proc = changes.first, on_new_proc = changes.second;
                    if (all_transmitions != 0 && double(transmitions - on_new_proc + on_curr_proc) / all_transmitions >= CR_U) {
                        continue;
                    }
                    transmitions = transmitions - on_new_proc + on_curr_proc;
                    proc[curr_tier] = new_proc;
                }
            }
        }
    }
    TierSchedule(const TierSchedule &schedule) {
        task = schedule.task;
        proc = schedule.proc;
        proc_load = schedule.proc_load;
        transmitions = schedule.transmitions;
        all_transmitions = schedule.all_transmitions;
    }
    double get_BF_value() { // invalid value if additional_criteria != BF
        return double(*std::max_element(proc_load.begin(), proc_load.end()) * proc_load.size()) / double(proc.size()) - 1;
    }
};

class TimeDiagram
{
public:
    std::vector<int> task_start;
    std::vector<int> task_finish;
    TimeDiagram(std::vector<int> &start, std::vector<int> &finish) {
        task_start = start;
        task_finish = finish;
    }
    int get_time() {
        return *std::max_element(task_finish.begin(), task_finish.end()); 
    }
};

TimeDiagram get_diagram(int task_num, int proc_num, TierSchedule &schedule, std::vector<std::vector<int>> &task_time, std::vector<std::vector<int>> &tran_time, std::vector<std::vector<int>> &parents)
{
    std::vector<int> task_start(task_num, -1); // start time for each task
    std::vector<int> task_finish(task_num, -1); // extra vector: finish time for each task
    std::vector<int> task_processor(task_num, -1); // number of process that has i task; can be initialized from `schedule`
    std::vector<int> times(proc_num, 0); // current finish time of each processor
    for (int i = 0; i < task_num; ++i) { // i is current tier
        int curr_proc = schedule.proc[i];
        int curr_task = schedule.task[i];
        int min_time = times[curr_proc]; // minimum time that is allowed for task to start from
        for (int p = 0; p < parents[curr_task].size(); ++p) { // pass through all parents to finds min_time
            int parent_task = parents[curr_task][p];
            int parent_proc = task_processor[parent_task];
            int total_time = task_finish[parent_task] + tran_time[parent_proc][curr_proc];
            if (min_time < total_time) {
                min_time = total_time;
            }
        }
        task_processor[curr_task] = curr_proc;
        task_start[curr_task] = min_time;
        task_finish[curr_task] = min_time + task_time[curr_proc][curr_task];
        times[curr_proc] = min_time + task_time[curr_proc][curr_task];
    }
    TimeDiagram diagram(task_start, task_finish);
    return diagram;
}

// TODO next_temperature for iterations when time of schedule doesn't change
double next_temperature(int iteration)
{
    switch (next_temperature_type) {
    case BOLTZMANN:
        return START_TEMPERATURE / std::log(1 + (iteration + 1));
    case CAUCHY:
        return START_TEMPERATURE / (1 + iteration);
    case LOGN_DIV_N:
        return START_TEMPERATURE * std::log(1 + (iteration + 1)) / (1 + (iteration + 1));
    }
    return 1;
}

int switch_processor(int proc_num, TierSchedule &schedule, int curr_tier, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents) // Operation 1
{
    int curr_proc = schedule.proc[curr_tier];
    int new_proc = rand() % (proc_num - 1);
    new_proc = new_proc < curr_proc ? new_proc : new_proc + 1;
    switch (additional_criteria) {
    case BF:
        schedule.proc_load[curr_proc]--; // check BF trying to change schedule
        schedule.proc_load[new_proc]++;
        if (schedule.get_BF_value() >= BF_U) {
            schedule.proc_load[curr_proc]++; // returns to old schedule 
            schedule.proc_load[new_proc]--;
            return 1; // operation declined
        }
    case CR:
        std::pair<int, int> changes = check_CR_change_by_switch_processor(curr_proc, new_proc, curr_tier, schedule.task, schedule.proc, graph, parents);
        int on_curr_proc = changes.first, on_new_proc = changes.second;
        if (schedule.all_transmitions != 0 && double(schedule.transmitions - on_new_proc + on_curr_proc) / schedule.all_transmitions >= CR_U) {
            return 1; // operation declined
        }
        schedule.transmitions = schedule.transmitions - on_new_proc + on_curr_proc;
    }
    schedule.proc[curr_tier] = new_proc;
    return 0; // operation completed successfully
}

int switch_tasks(int task_num, int proc_num, TierSchedule &schedule, int curr_tier, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents) // Operation 2
{
    int curr_task = schedule.task[curr_tier];
    int curr_proc = schedule.proc[curr_tier];
    std::vector<int> allowed_ind; // vector of indexes which can be used to switch curr_task
    for (int i = 0; i < task_num; ++i) {
        if (std::find(graph[curr_task].begin(), graph[curr_task].end(), schedule.task[i]) != graph[curr_task].end()) { // if we found descedant
            break;
        }
        if (std::find(parents[curr_task].begin(), parents[curr_task].end(), schedule.task[i]) != parents[curr_task].end()) { // if we found parent
            allowed_ind.clear();
        } else if (i != curr_tier && schedule.proc[i] == curr_proc) {
            allowed_ind.push_back(i);   
        }
    }
    if (allowed_ind.size() == 0) {
        return 1;
    }
    int new_ind = allowed_ind[rand() % allowed_ind.size()];
    schedule.task.erase(schedule.task.begin() + curr_tier);
    schedule.proc.erase(schedule.proc.begin() + curr_tier);
    schedule.task.insert(schedule.task.begin() + new_ind, curr_task);
    schedule.proc.insert(schedule.proc.begin() + new_ind, curr_proc);
    return 0; // TODO add statistics of operation drops, number of iterations summary, others 
              // done statistics: skipped_iterations(drops)
}

void transform_schedule(int task_num, int proc_num, TierSchedule &schedule, TierSchedule &new_schedule, int temperature, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents)
{
    int curr_tier = rand() % task_num;
    if (rand() % 2) {
        // Operation 1
        // Returns 1 if nothing changed
        skipped_iterations += switch_processor(proc_num, new_schedule, curr_tier, graph, parents);
    } else {
        // Operation 2
        // Returns 1 if nothing changed
        skipped_iterations += switch_tasks(task_num, proc_num, new_schedule, curr_tier, graph, parents);
    }
}

int main()
{
    /// Initialization
    int task_num; // N
    std::cin >> task_num;
    int proc_num; // S
    std::cin >> proc_num;
    std::vector<std::vector<int>> task_time(proc_num, std::vector<int>(task_num, 0)); // C
    for (int i = 0; i < task_time.size(); ++i) {
        for (int j = 0; j < task_time[i].size(); ++j) {
            std::cin >> task_time[i][j];
        }
    }
    std::vector<std::vector<int>> tran_time(proc_num, std::vector<int>(proc_num, 0)); // D
    for (int i = 0; i < tran_time.size(); ++i) {
        for (int j = 0; j < tran_time[i].size(); ++j) {
            std::cin >> tran_time[i][j];
        }
    }
    std::vector<std::vector<int>> graph(task_num, std::vector<int>()); // list of arcs
    std::vector<std::vector<int>> parents(task_num, std::vector<int>());
    for (int i = 0; i < task_num; ++i) {
        for (int j = 0; j < task_num; ++j) {
            int r;
            std::cin >> r;
            if (r) {
                graph[i].push_back(j);
                parents[j].push_back(i);
            } 
        }
    }
    
    /// First correct schedule
    TierSchedule schedule(task_num, proc_num, graph, parents);

    /// Simulated annealing algorithm
    int best_iteration = 0;
    int best_time = get_diagram(task_num, proc_num, schedule, task_time, tran_time, parents).get_time();
    int curr_time = best_time;
    if (OUT != CSV) {
        std::cout << "First time: " << curr_time << std::endl;
    }
    TierSchedule best_schedule(schedule);
    double temperature = START_TEMPERATURE;
    for (int k = 0; k < ITERATION_NUMBER; ++k) {
        temperature = next_temperature(k); 
        for (int its = 0; its < TEMPERATURE_ITERATIONS; ++its) {
            TierSchedule new_schedule(schedule);
            transform_schedule(task_num, proc_num, schedule, new_schedule, temperature, graph, parents);
            int new_time = get_diagram(task_num, proc_num, new_schedule, task_time, tran_time, parents).get_time();
            if (best_time >= new_time) {
                schedule = new_schedule;
                best_schedule = new_schedule;
                best_time = new_time;
                curr_time = new_time;
            } else if (curr_time >= new_time) {
                schedule = new_schedule;
                curr_time = new_time;
            } else {
                double prob = (double) rand() / RAND_MAX;
                if (prob <= exp((double) (curr_time - new_time) / temperature)) {
                    schedule = new_schedule;
                    curr_time = new_time;
                }
            }
        }
    }
    
    /// Output best diagram
    TimeDiagram best_diagram = get_diagram(task_num, proc_num, best_schedule, task_time, tran_time, parents);
    if (OUT == NO_OUTPUT) {
        std::cout << "Best time: " << best_diagram.get_time() << std::endl;
        std::cout << "Skipped iterations: " << skipped_iterations << std::endl;
        switch (additional_criteria) {
        case BF:
            std::cout << "BF criteria number: " << best_schedule.get_BF_value() << std::endl;
        case CR:
            if (schedule.all_transmitions != 0) {
                std::cout << "CR criteria number: " << double(best_schedule.transmitions) / best_schedule.all_transmitions << std::endl;
            }
        }
        return 0;
    }
    if (OUT != CSV) {
        for (int i = 0; i < task_num; ++i) {
            std::cout << "Task " << best_schedule.task[i] << " on processor " << best_schedule.proc[i] << std::endl;
            std::cout << "From " << best_diagram.task_start[best_schedule.task[i]] << " to " << best_diagram.task_finish[best_schedule.task[i]] << std::endl;
            std::cout << std::endl;
        }
        std::cout << "End temperatire: " << temperature << std::endl;
        std::cout << "Best time: " << best_diagram.get_time() << std::endl;
        std::cout << "Skipped iterations: " << skipped_iterations << std::endl;
    }
    if (OUT == CSV) {
        std::vector<std::vector<int>> csv_out(proc_num, std::vector<int>(best_diagram.get_time(), -1));
        for (int i = 0; i < task_num; ++i) {
            for (int j = best_diagram.task_start[best_schedule.task[i]]; j < best_diagram.task_finish[best_schedule.task[i]]; ++j) {
                csv_out[best_schedule.proc[i]][j] = best_schedule.task[i];
            }
        }
        for (int i = 0; i < proc_num; ++i) {
            for (int j = 0; j < best_diagram.get_time() - 1; ++j) {
                std::cout << csv_out[i][j] << ",";
            }
            std::cout << csv_out[i][best_diagram.get_time() - 1] << std::endl;
        }
    }

    return 0;
}