#include <iostream>
#include <vector>
#include <algorithm>

#define ITERATION_NUMBER 1000
#define START_TEMPERATURE 50

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

// Finds first index in vector, that is not equal to `N`
int find_ind(int N, std::vector<int> &curr)
{
    for (int i = 0; i < curr.size(); ++i) {
        if (curr[i] != N) {
            return i;
        }
    }
    return -1;
}

// Now it returns time of schedule
// mb return a lot of stuff: diargam, max time ... 
int transform_to_diagram(int task_num, int proc_num, std::vector<std::vector<int>> &schedule, std::vector<std::vector<int>> &task_time, std::vector<std::vector<int>> &tran_time, std::vector<std::vector<int>> &parents)
{
    std::vector<int> task_start(task_num, -1); // start time for each task
    std::vector<int> task_finish(task_num, -1); // extra vector: finish time for each task
    std::vector<int> task_processor(task_num, -1); // number of process that has i task; can be initialized from `schedule`
    std::vector<int> times(proc_num, 0); // current finish time of each processor
    for (int i = 0; i < schedule.size(); ++i) { // i is current tier
        int curr_proc = find_ind(-1, schedule[i]); // curr_proc is index on which processor task is
        int curr_task = schedule[i][curr_proc];
        int min_time = times[curr_proc]; // minimum time that is allowed for task to start from
        for (int p = 0; p < parents[curr_task].size(); ++p) { // pass through all parents to finds min_time
            int parent_task = parents[curr_task][p];
            int parent_proc = task_processor[parent_task];
            int total_time = task_start[parent_task] + task_time[parent_proc][parent_task] + tran_time[parent_proc][curr_proc];
            if (min_time < total_time) {
                min_time = total_time;
            }
        }
        task_processor[curr_task] = curr_proc;
        task_start[curr_task] = min_time;
        task_finish[curr_task] = min_time + task_time[curr_proc][curr_task];
        times[curr_proc] = min_time + task_time[curr_proc][curr_task];
    }

    /*
    print(schedule);
    for (auto e : task_start) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
    for (auto e : task_finish) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
    for (auto e : task_processor) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
    for (auto e : times) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
    */

    return *std::max_element(times.begin(), times.end());
}



int main()
{
    /// Initialization
    int task_num; // N
    std::cin >> task_num;
    int proc_num;   // S
    std::cin >> proc_num;
    // MB change type of task_time and tran_time
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
    std::vector<std::vector<int>> schedule(task_num, std::vector<int>(proc_num, -1)); // tier view of max height: i is tier, j is processor
    // sort tasks
    std::vector<int> sorted = topology_sort(graph);
    // put tasks in random processors
    srand(2021);
    for (int i = 0; i < task_num; ++i) {
        schedule[i][rand() % proc_num] = sorted[i];
    }

    /// Simulated annealing algorithm
    int record = transform_to_diagram(task_num, proc_num, schedule, task_time, tran_time, parents);
    

    return 0;
}