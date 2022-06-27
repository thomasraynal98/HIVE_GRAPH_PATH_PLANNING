#include "useful.h"
#include <OpenXLSX.hpp>
#include <bits/stdc++.h>

void Read_XLSX_file(std::string path, std::vector<Data_node>& vector_node, std::vector<Data_road>& road_vector)
{
    OpenXLSX::XLDocument doc;
    doc.open(path);

    // ADD NODE.
    auto wks = doc.workbook().worksheet("GEO_POINT");

    for(auto& row : wks.rows())
    {
        bool end = false;
        int i = 0;

        int id = -1;
        double longitude = 0;
        double latitude = 0;

        for(auto cell : row.cells(3))
        {
            if(cell.value().type() == OpenXLSX::XLValueType::Empty) break;

            end = true;
            if(i == 0) id        = cell.value();
            if(i == 1) longitude = cell.value();
            if(i == 2) latitude  = cell.value();
            i++;
        }

        if(!end) break;

        Data_node new_data(id, longitude, latitude);
        vector_node.push_back(new_data);
    }

    // ADD ROAD.
    auto wks2 = doc.workbook().worksheet("PATH");

    for(auto& row : wks2.rows())
    {
        bool end = false;
        int i = 0;

        int id_road = -1;
        int id_pointA = 0;
        int id_pointB = 0;
        double deg_to_A = 0;
        double deg_to_B = 0;
        double length = 0;
        bool available = true;
        double speed = 0;

        for(auto cell : row.cells(8))
        {
            if(cell.value().type() == OpenXLSX::XLValueType::Empty) break;

            end = true;
            if(i == 0) id_road   = cell.value();
            if(i == 1) id_pointA = cell.value();
            if(i == 2) id_pointB = cell.value();
            if(i == 3) deg_to_A  = cell.value();
            if(i == 4) deg_to_B  = cell.value();
            if(i == 5) length    = cell.value();
            if(i == 6)
            {
                int tempo = cell.value();
                if(tempo == 1) available = true;
                else{available = false;}
            }
            if(i == 7) speed     = cell.value();
            i++;
        }

        if(!end) break;

        Data_node* tempo_save;
        for(int i = 0; i < vector_node.size(); i++)
        {
            if(id_pointA == vector_node[i].node_ID) { tempo_save = &vector_node[i]; break;}
        }
        for(int i = 0; i < vector_node.size(); i++)
        {
            if(id_pointB == vector_node[i].node_ID)
            {
                Data_road new_road(id_road, tempo_save, &vector_node[i]);
                new_road.available = available;
                new_road.deg_to_A = deg_to_A;
                new_road.deg_to_B = deg_to_B;
                new_road.length = length;
                new_road.max_speed = speed;
                road_vector.push_back(new_road);
            }
        }
    }
    doc.close();
}

void Read_YAML_file(std::string path, std::vector<Geographic_point>* ref_border)
{
    cv::FileStorage fsSettings(path, cv::FileStorage::READ);
    if(!fsSettings.isOpened())
    {
        std::cerr << "ERROR: Can't open the file " << path << std::endl;
        return;
    }

    std::string information;
    size_t size = 10;

    Geographic_point p1(0,0);
    fsSettings["p1_longitude"] >> information;
    p1.longitude = std::stod(information, &size);
    fsSettings["p1_latitude"] >> information;
    p1.latitude = std::stod(information);

    Geographic_point p2(0,0);
    fsSettings["p2_longitude"] >> information;
    p2.longitude = std::stod(information);
    fsSettings["p2_latitude"] >> information;
    p2.latitude = std::stod(information);

    ref_border->push_back(p1);
    ref_border->push_back(p2);
}

void Write_XLSX_file(std::string path, std::vector<Data_node>& node_vector, std::vector<Data_road>& road_vector)
{
    OpenXLSX::XLDocument doc;
    doc.open(path);
    auto wks = doc.workbook().worksheet("PATH");

    // CLEAR THE FILE
    for(auto& row : wks.rows())
    {
        for(auto cell : row.cells(8))
        {
            cell.value().clear();
        }
    }

    // ADD ROAD
    int index = 1;
    for(auto road : road_vector)
    {
        std::string id_cell = "A" + std::to_string(index);
        wks.cell(id_cell).value() = road.road_ID;
        id_cell = "B" + std::to_string(index);
        wks.cell(id_cell).value() = road.A->node_ID;
        id_cell = "C" + std::to_string(index);
        wks.cell(id_cell).value() = road.B->node_ID;
        id_cell = "D" + std::to_string(index);
        wks.cell(id_cell).value() = road.deg_to_A;
        id_cell = "E" + std::to_string(index);
        wks.cell(id_cell).value() = road.deg_to_B;
        id_cell = "F" + std::to_string(index);
        wks.cell(id_cell).value() = road.length;
        id_cell = "G" + std::to_string(index);
        wks.cell(id_cell).value() = (double)(road.available);
        id_cell = "H" + std::to_string(index);
        wks.cell(id_cell).value() = road.max_speed;
        index++;
    }

    doc.save();

    // ADD POINT
    
    doc.close();
}

void Read_JPG_file(std::string path, cv::Mat& img)
{
    img = imread(path, cv::IMREAD_COLOR);
    if(img.empty())
    {
        std::cout << "Could not read the image: " << path << std::endl;
    }
}

void Init_data_map(cv::Mat& map_current, cv::Mat& map_data)
{
    cv::Mat new_map_data(map_current.rows, map_current.cols, CV_16UC1, cv::Scalar(0));
    map_data = new_map_data;
}

void Project_all_element(std::vector<Geographic_point>& ref_border, std::vector<Data_node>& node_vector, cv::Mat& map_current, cv::Mat& map_data, std::vector<Data_road>& road_vector, bool speed_view)
{    
    // Draw road.
    for(auto& road : road_vector)
    { 
        if(speed_view)
        {
            if(road.max_speed < 6) cv::line(map_current, cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(180, 180, 180), 10, cv::LINE_8);
            if(road.max_speed > 6 && road.max_speed < 8) cv::line(map_current, cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(102, 102, 255), 10, cv::LINE_8);
            if(road.max_speed > 8) cv::line(map_current, cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(51, 51, 255), 10, cv::LINE_8);
        }
        if(road.available)
        {
            cv::line(map_current, cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(182,242,176), 4, cv::LINE_8);
        }
        else
        {
            cv::line(map_current, cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(255, 0, 127), 4, cv::LINE_8);
        }
        cv::line(map_data   , cv::Point((int)(road.A->col_idx),(int)(road.A->row_idx)), cv::Point((int)(road.B->col_idx),(int)(road.B->row_idx)), cv::Scalar(road.road_ID), 4, cv::LINE_8);
    }

    // Draw node.
    for(auto& node : node_vector)
    {   
        node.col_idx = ((node.point.longitude - ref_border[0].longitude) * (double)(map_current.cols)) / (ref_border[1].longitude - ref_border[0].longitude);
        node.row_idx = (double)(map_current.rows) - (((node.point.latitude - ref_border[1].latitude) * (double)(map_current.rows)) / (ref_border[0].latitude - ref_border[1].latitude));
        
        cv::circle(map_current, cv::Point((int)(node.col_idx),(int)(node.row_idx)),5,      cv::Scalar(0,0,250), cv::FILLED, 1,0);
        cv::circle(map_data,    cv::Point((int)(node.col_idx),(int)(node.row_idx)),5, cv::Scalar(node.node_ID), cv::FILLED, 1,0);
    }
}

void clear_road_vector(std::vector<Data_road>& road_vector)
{
    road_vector.clear();
}

long double toRadians(const long double degree)
{
    long double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}

double compute_distance_to_end(Data_node A, Data_node B)
{
    // Distance.
    double lat1 = toRadians(A.point.latitude);
    double long1 = toRadians(A.point.longitude);
    double lat2 = toRadians(B.point.latitude);
    double long2 = toRadians(B.point.longitude);
    // Haversine Formula
    long double dlong = long2 - long1;
    long double dlat = lat2 - lat1;

    long double ans = pow(sin(dlat / 2), 2) +
                        cos(lat1) * cos(lat2) *
                        pow(sin(dlong / 2), 2);

    ans = 2 * asin(sqrt(ans));
    long double R = 6371;
    return ans * R;
}

double compute_weight_road(Data_road* road)
{
    return road->length / road->max_speed; // en heure decimal.
}

void fill_path_node(std::vector<Data_node>& vector_node, std::vector<Data_road>& road_vector, std::vector<Path_node>& graph)
{
    for(int idx_node = 0; idx_node < vector_node.size(); idx_node++)
    {
        Path_node new_path_node(vector_node[idx_node].node_ID, &vector_node[idx_node]);
        graph.push_back(new_path_node);
    }
    for(int idx_road = 0; idx_road < road_vector.size(); idx_road++)
    {
        for(int idx_path = 0; idx_path < graph.size(); idx_path++)
        {
            if(graph[idx_path].index_node == road_vector[idx_road].A->node_ID)
            {
                // 1 FOUND PATH NODE CONNECTION
                for(int idx_path2 = 0; idx_path2 < graph.size(); idx_path2++)
                {
                    if(graph[idx_path2].index_node == road_vector[idx_road].B->node_ID)
                    {
                        graph[idx_path].connection_data.push_back(&graph[idx_path2]);
                        graph[idx_path2].connection_data.push_back(&graph[idx_path]);
                        // 2 COMPUTE WEIGHT
                        graph[idx_path].connection_weight.push_back(compute_weight_road(&road_vector[idx_road]));
                        graph[idx_path2].connection_weight.push_back(compute_weight_road(&road_vector[idx_road]));
                    }
                }
            }
            // if(graph[idx_path].index_node == road_vector[idx_road].B->node_ID)
            // {
            //     // 1 FOUND PATH NODE CONNECTION
            //     for(int idx_path2 = 0; idx_path2 < graph.size(); idx_path2++)
            //     {
            //         if(graph[idx_path2].index_node == road_vector[idx_road].A->node_ID)
            //         {
            //             graph[idx_path].connection_data.push_back(&graph[idx_path2]);
            //             graph[idx_path2].connection_data.push_back(&graph[idx_path]);
            //             // 2 COMPUTE WEIGHT
            //             graph[idx_path].connection_weight.push_back(compute_weight_road(&road_vector[idx_road]));
            //             graph[idx_path2].connection_weight.push_back(compute_weight_road(&road_vector[idx_road]));
            //         }
            //     }
            // }
        }
    }
}