#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "gurobi_c++.h"

#include "globals.cpp"
#include "Request.cpp"
#include "Vehicle.cpp"
#include "RV.cpp"
#include "RTV.cpp"
#include "GPtree.cpp"
#include "util.cpp"
using namespace std;


int main() {
    
    GRBEnv *env = new GRBEnv();
    now_time = 0;

    map<pair<int, int>, int> *dist = new map<pair<int, int>, int>;
    initialize(true);
    printf("load_end\n\n");

    vector<Vehicle> vehicles;
    vehicles.reserve(max_vehicle);
    read_vehicles("input/vehicles.csv", vehicles);

    vector<Request> requests;
    requests.reserve(100);

    bool hasMore = false;
    Request tail;

    vector<Request> unserved;

    FILE *in = get_requests_file("input/part_reqs.csv");

    while (true) { // TODO last update?
        now_time += time_step;

        requests.clear();
        if (hasMore) {
            requests.push_back(tail);
        }

        handle_unserved(unserved, requests, now_time);
        if (read_requests(in, requests, now_time, dist)) {
            tail = requests.back();
            requests.pop_back();
            hasMore = true;
        } else {
            hasMore = false;
        }
        update_vehicles(vehicles, requests, now_time);

        printf("Building RV graph...\n");
        RVGraph *RV = new RVGraph(vehicles, requests, dist);
        printf("RV graph builded!\n\n");

        printf("Building RTV graph...\n");
        RTVGraph *RTV = new RTVGraph(RV, vehicles, requests, dist);
        printf("RTV graph builded!\n\n");

        printf("Start to solve ILP...\n");
        RTV->solve(env, vehicles, requests, unserved, dist);
        printf("\nSuccess!\n\n");

        delete RV;
        delete RTV;

        if (!hasMore) {
            break;
        }
    }

    fclose(in);

    delete dist;
    delete env;

    return 0;
}