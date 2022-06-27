/*
    We use Dijkstra's Shortest Path Algorithm to find the good direction to take.
*/

#include "useful.h"

std::vector<Data_node> node_vector;
std::vector<Data_road> road_vector;
std::vector<Path_node> graph;

void show_full_path(Path_node* next_node, int idx_start, std::vector<Data_road> road_vector)
{
    double distance_km = 0;
    double time_total_decimal = 0;
    while(next_node->come_from != NULL && next_node->index_node != idx_start)
    {
        std::cout << " NODE " << next_node->index_node << std::endl;

        // FOUND ROAD BETWEEN.
        for(int i = 0; i < road_vector.size(); i++)
        {
            if(road_vector[i].A->node_ID == next_node->index_node && \
            road_vector[i].B->node_ID == next_node->come_from->index_node)
            {
                std::cout << " ROAD " << road_vector[i].road_ID << std::endl;
                distance_km += road_vector[i].length;
                time_total_decimal += (road_vector[i].length/road_vector[i].max_speed);
            }
            if(road_vector[i].B->node_ID == next_node->index_node && \
            road_vector[i].A->node_ID == next_node->come_from->index_node)
            {
                std::cout << " ROAD " << road_vector[i].road_ID << std::endl;
                distance_km += road_vector[i].length;
                time_total_decimal += (road_vector[i].length/road_vector[i].max_speed);
            }
        }

        next_node = next_node->come_from;
    }
    std::cout << " NODE " << idx_start << std::endl;
    std::cout << std::setprecision(3);
    std::cout << "DISTANCE TOTAL : " << distance_km << " KM (" << std::ceil(time_total_decimal*60) << "Min)" << std::endl;
}

int main()
{
    std::cout << std::setprecision(10);

    Read_XLSX_file("../data/Hive_Map_Database2.xlsx", node_vector, road_vector);

    fill_path_node(node_vector, road_vector, graph); 

    // for(auto all : graph)
    // {
    //     std::cout << all.index_node << " " << all.n << " " << all.closet << " " << all.fscore << " " << \
    //     all.gscore << " " << all.come_from << std::endl;
    //     for(int i = 0; i < all.connection_data.size(); i++)
    //     {
    //         std::cout << all.connection_data[i]->index_node << " " << all.connection_weight[i] << std::endl;
    //     }
    // }

    std::priority_queue<TuplePath, std::vector<TuplePath>,std::greater<TuplePath> > openList;   

    // INITIALISATION START
    int idx_start = 1;
    int idx_endof = 14;
    Path_node* start_node;
    Path_node* endof_node;

    for(int idx_graph = 0; idx_graph < graph.size(); idx_graph++)
    {
        if(graph[idx_graph].index_node == idx_start)
        {
            start_node = &graph[idx_graph];
            start_node->gscore = 0;
        }
        if(graph[idx_graph].index_node == idx_endof)
        {
            endof_node = &graph[idx_graph];
        }
    }
    
    TuplePath start_tuple(0.0, start_node);
    openList.emplace(start_tuple);

    while(!openList.empty())
    {
        TuplePath p = openList.top();
        std::get<1>(p)->closet = true;
        openList.pop();

        // POUR CHAQUE VOISIN
        for(int idx_voisin = 0; idx_voisin < std::get<1>(p)->connection_weight.size(); idx_voisin++)
        {
            // // VERIFIER SI ON EST A DESTINATION
            if(std::get<1>(p)->connection_data[idx_voisin]->index_node == endof_node->index_node)
            {
                std::get<1>(p)->connection_data[idx_voisin]->come_from = std::get<1>(p);
                Path_node* next_node = std::get<1>(p)->connection_data[idx_voisin];

                while(next_node->come_from != NULL && next_node->index_node != idx_start)
                {
                    // std::cout << " NODE " << next_node->index_node << std::endl;
                    next_node = next_node->come_from;
                }
                // std::cout << " NODE " << start_node->index_node << std::endl;

                show_full_path(std::get<1>(p)->connection_data[idx_voisin], idx_start, road_vector);
                // std::cout << "PATH FOUND" << std::endl;

                return 0;
            }

            //  // CALCULER LE TENTATIVE_GSCORE = GSCORE[CURRENT] + DEPLACEMENT[CURRENT<>VOISIN]
            double tentative_gscode = std::get<1>(p)->gscore + std::get<1>(p)->connection_weight[idx_voisin];

            //  // IF TENTATIVE_GSCORE < GSCORE[VOISIN] //SETUP A 99999 DE BASE
            if(tentative_gscode < std::get<1>(p)->connection_data[idx_voisin]->gscore)
            {
                //  //  // VOISIN PARENT = CURRENT
                std::get<1>(p)->connection_data[idx_voisin]->come_from = std::get<1>(p);

                //  //  // GSCORE[VOISIN] = TENTATIVE_GSCORE
                std::get<1>(p)->connection_data[idx_voisin]->gscore = tentative_gscode;

                //  //  // FSCORE[VOISIN] = TENTATIVE_GSCORE + DISTANCE[VOISIN<>DESTINATION]
                std::get<1>(p)->connection_data[idx_voisin]->fscore = tentative_gscode + compute_distance_to_end(*endof_node->n, *std::get<1>(p)->connection_data[idx_voisin]->n);

                //  //  // IF VOISIN N'A PAS ENCORE ETAIT VISITER [CLOSET=FALSE]
                if(!std::get<1>(p)->connection_data[idx_voisin]->closet)
                {
                    //  //  //  // AJOUT VOISIN DANS OPENLIST
                    openList.emplace(std::get<1>(p)->connection_data[idx_voisin]->fscore, std::get<1>(p)->connection_data[idx_voisin]);
                }
            }
        }
    }

    std::cout << "NO PATH FOUND" << std::endl;

    return 0;
}