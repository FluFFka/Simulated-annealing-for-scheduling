#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#define ITERATION_NUMBER 50000
#define START_TEMPERATURE 100

enum {
    BOLTZMANN,
    CAUCHY,
    LOGN_DIV_N,
};

int next_temperature_type = BOLTZMANN;

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

TimeDiagram get_diagram(int task_num, int proc_num, std::vector<int> &schedule_task, std::vector<int> &schedule_proc, std::vector<std::vector<int>> &task_time, std::vector<std::vector<int>> &tran_time, std::vector<std::vector<int>> &parents)
{
    std::vector<int> task_start(task_num, -1); // start time for each task
    std::vector<int> task_finish(task_num, -1); // extra vector: finish time for each task
    std::vector<int> task_processor(task_num, -1); // number of process that has i task; can be initialized from `schedule`
    std::vector<int> times(proc_num, 0); // current finish time of each processor
    for (int i = 0; i < task_num; ++i) { // i is current tier
        //int curr_proc = find_ind(-1, schedule[i]); // curr_proc is index on which processor task is
        int curr_proc = schedule_proc[i];
        int curr_task = schedule_task[i];
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
    TimeDiagram diagram(task_start, task_finish);
    return diagram;
}

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

void switch_processor(int proc_num, std::vector<int> &schedule_proc, int curr_tier) // Operation 1
{
    int curr_proc = schedule_proc[curr_tier];
    int new_proc = rand() % (proc_num - 1);
    new_proc = new_proc < curr_proc ? new_proc : new_proc + 1;
    schedule_proc[curr_tier] = new_proc;
}

int switch_tasks(int task_num, int proc_num, std::vector<int> &schedule_task, std::vector<int> &schedule_proc, int curr_tier, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents) // Operation 2
{
    int curr_task = schedule_task[curr_tier];
    int curr_proc = schedule_proc[curr_tier];
    std::vector<int> allowed_ind; // vector of indexes which can be used to switch curr_task
    for (int i = 0; i < task_num; ++i) {
        if (std::find(graph[curr_task].begin(), graph[curr_task].end(), schedule_task[i]) != graph[curr_task].end()) { // if we found descedant
            break;
        }
        if (std::find(parents[curr_task].begin(), parents[curr_task].end(), schedule_task[i]) != parents[curr_task].end()) { // if we found parent
            allowed_ind.clear();
        } else if (i != curr_tier && schedule_proc[i] == curr_proc) {
            allowed_ind.push_back(i);   
        }
    }
    if (allowed_ind.size() == 0) {
        return 0;
    }
    int new_ind = allowed_ind[rand() % allowed_ind.size()];
    schedule_task.erase(schedule_task.begin() + curr_tier);
    schedule_proc.erase(schedule_proc.begin() + curr_tier);
    schedule_task.insert(schedule_task.begin() + new_ind, curr_task);
    schedule_proc.insert(schedule_proc.begin() + new_ind, curr_proc);
    return 1;
}

void transform_schedule(int task_num, int proc_num, std::vector<int> &schedule_task, std::vector<int> &schedule_proc, std::vector<int> &new_schedule_task, std::vector<int> &new_schedule_proc, int temperature, std::vector<std::vector<int>> &graph, std::vector<std::vector<int>> &parents)
{
    for (int i = 0; i < temperature; ++i) {
        int curr_tier = rand() % task_num;
        if (rand() % 2) {
            // Operation 1
            switch_processor(proc_num, new_schedule_proc, curr_tier);
        } else {
            // Operation 2
            // Returns 0 if nothing changed
            switch_tasks(task_num, proc_num, new_schedule_task, new_schedule_proc, curr_tier, graph, parents);
        }
    }
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
    std::vector<int> schedule_task = topology_sort(graph);
    std::vector<int> schedule_proc(task_num);
    // put tasks in random processors
    srand(2021);
    for (int i = 0; i < task_num; ++i) {
        schedule_proc[i] = rand() % proc_num;
    }

    /// Simulated annealing algorithm
    int best_iteration = 0;
    int best_time = get_diagram(task_num, proc_num, schedule_task, schedule_proc, task_time, tran_time, parents).get_time();
    int curr_time = best_time;
    std::cout << "First time: " << curr_time << std::endl;
    std::vector<int> best_schedule_task = schedule_task;
    std::vector<int> best_schedule_proc = schedule_proc;
    double temperature = START_TEMPERATURE;
    for (int k = 0; k < ITERATION_NUMBER; ++k) {
        temperature = next_temperature(k);
        std::vector<int> new_schedule_task = schedule_task;
        std::vector<int> new_schedule_proc = schedule_proc;
        transform_schedule(task_num, proc_num, schedule_task, schedule_proc, new_schedule_task, new_schedule_proc, temperature, graph, parents);
        int new_time = get_diagram(task_num, proc_num, new_schedule_task, new_schedule_proc, task_time, tran_time, parents).get_time();
        // TODO decrease assignments
        if (best_time >= new_time) {
            schedule_task = new_schedule_task;
            best_schedule_task = new_schedule_task;
            schedule_proc = new_schedule_proc;
            best_schedule_proc = new_schedule_proc;
            best_time = new_time;
            curr_time = new_time;
        } else if (curr_time >= new_time) {
            schedule_task = new_schedule_task;
            schedule_proc = new_schedule_proc;
            curr_time = new_time;
        } else {
            double prob = (double) rand() / RAND_MAX;
            //std::cout << prob << " " << curr_time << " " << new_time << " " << temperature << " " << exp((curr_time - new_time) / temperature) << std::endl;
            if (prob <= exp((double) (curr_time - new_time) / temperature)) {
                schedule_task = new_schedule_task;
                schedule_proc = new_schedule_proc;
                curr_time = new_time;
            }
        }
    }
    
    /// Output best diagram
    TimeDiagram best_diagram = get_diagram(task_num, proc_num, best_schedule_task, best_schedule_proc, task_time, tran_time, parents);
    std::cout << "Best time: " << best_diagram.get_time() << std::endl;
    for (int i = 0; i < task_num; ++i) {
        std::cout << "Task " << best_schedule_task[i] << " on processor " << best_schedule_proc[i] << std::endl;
        std::cout << "From " << best_diagram.task_start[best_schedule_task[i]] << " to " << best_diagram.task_finish[best_schedule_task[i]] << std::endl;
        std::cout << std::endl;
    }
    // TODO output into csv for better view
    return 0;
}