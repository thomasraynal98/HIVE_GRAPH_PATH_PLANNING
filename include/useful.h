#include <string.h>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

struct Opencv_area
{
    int top_left_col;
    int top_left_row;
    int down_right_col;
    int down_right_row;

    Opencv_area(int a, int b, int c, int d)
        : top_left_col(a)
        , top_left_row(b)
        , down_right_col(c)
        , down_right_row(d)
        {}
    
    bool is_in(int idx_col, int idx_row)
    {
        if(top_left_col < idx_col && idx_col < down_right_col && top_left_row < idx_row && idx_row < down_right_row) return true;

        return false;
    }

    void fullfill(int a, int b, int c, int d){
        top_left_col = a;
        top_left_row = b;
        down_right_col = c;
        down_right_row = d;
    }

    int option_selection(int idx_col, int idx_row)
    {
        if(top_left_col < idx_col && idx_col < top_left_col+30 && top_left_row < idx_row && idx_row < top_left_row+30) return 1;
        if(top_left_col+30 < idx_col && idx_col < top_left_col+60 && top_left_row < idx_row && idx_row < top_left_row+30) return 2;
        if(top_left_col+60 < idx_col && idx_col < top_left_col+90 && top_left_row < idx_row && idx_row < top_left_row+30) return 6;
        if(top_left_col < idx_col && idx_col < top_left_col+30 && top_left_row+30 < idx_row && idx_row < top_left_row+60) return 3;
        if(top_left_col+30 < idx_col && idx_col < top_left_col+60 && top_left_row+30 < idx_row && idx_row < top_left_row+60) return 4;
        if(top_left_col+60 < idx_col && idx_col < top_left_col+90 && top_left_row+30 < idx_row && idx_row < top_left_row+60) return 5;
    }
};

struct Geographic_point
{
    double longitude, latitude;

    Geographic_point(double a, double b)
        : longitude(a)
        , latitude(b)
        {}

    void operator=(Geographic_point& pixel2)
    {
        longitude = pixel2.longitude;
        latitude   = pixel2.latitude;
    }
};

struct Data_node
{
    Geographic_point point;
    int node_ID;
    double col_idx, row_idx;

    Data_node(int a, double b, double c)
        : point(b,c)
        , node_ID(a)
        , col_idx(0)
        , row_idx(0)
        {}


    void operator=(Data_node& pixel2)
    {
        node_ID = pixel2.node_ID;
        point   = pixel2.point;
    }
};

struct Data_road
{
    int road_ID;
    Data_node* A;
    Data_node* B;
    double deg_to_A, deg_to_B;
    double length;
    bool available;
    double max_speed;

    Data_road(int a, Data_node* b, Data_node* c)
        : road_ID(a)
        , A(b)
        , B(c)
        , deg_to_A(0.1)
        , deg_to_B(0.1)
        {init_data_road();}

    long double toRadians(const long double degree)
    {
        // cmath library in C++
        // defines the constant
        // M_PI as the value of
        // pi accurate to 1e-30
        long double one_deg = (M_PI) / 180;
        return (one_deg * degree);
    }

    void init_data_road()
    {
        max_speed = 7.001;
        available = true;
        
        // Calcul distance between point.
        double d = B->point.longitude - A->point.longitude;
        double x = cos(B->point.latitude) * sin(d);
        double y = cos(A->point.latitude) * sin(B->point.latitude) - (sin(A->point.latitude) * cos(B->point.latitude) * cos(d));
        deg_to_B = atan2(x,y) * 180 / 3.14;
        if(deg_to_B < 0) deg_to_B = 180 + (180 + deg_to_B);

        deg_to_A = deg_to_B + 180;
        if(deg_to_A > 360) deg_to_A = deg_to_A - 360;

        // Distance.
        double lat1 = toRadians(A->point.latitude);
        double long1 = toRadians(A->point.longitude);
        double lat2 = toRadians(B->point.latitude);
        double long2 = toRadians(B->point.longitude);
        // Haversine Formula
        long double dlong = long2 - long1;
        long double dlat = lat2 - lat1;
    
        long double ans = pow(sin(dlat / 2), 2) +
                            cos(lat1) * cos(lat2) *
                            pow(sin(dlong / 2), 2);
    
        ans = 2 * asin(sqrt(ans));
        long double R = 6371;
        length = ans * R;
    }
};


struct Path_node{
    int index_node;
    Data_node* n;
    bool closet;
    double fscore;
    double gscore;
    std::vector<Path_node*> connection_data;
    std::vector<double> connection_weight;
    Path_node* come_from; 

    Path_node(int id, Data_node* a)
        : n(a)
        , closet(false)
        , fscore(99999)
        , gscore(99999)
        , index_node(id)
        , come_from(NULL)
        {}
};

typedef std::tuple<double, Path_node*> TuplePath;

void Read_XLSX_file(std::string path, std::vector<Data_node>& vector_node, std::vector<Data_road>& road_vector);
void Read_YAML_file(std::string path, std::vector<Geographic_point>* info_brut);
void Write_XLSX_file(std::string path,std::vector<Data_node>& node_vector, std::vector<Data_road>& road_vector);
void Read_JPG_file(std::string path, cv::Mat& img);
void Init_data_map(cv::Mat& map_current, cv::Mat& map_data);
void Project_all_element(std::vector<Geographic_point>& ref_border, std::vector<Data_node>& node_vector, cv::Mat& map_current, cv::Mat& map_data, std::vector<Data_road>& road_vector, bool speed_view);
void clear_road_vector(std::vector<Data_road>& road_vector);
long double toRadians(const long double degree);
double compute_distance_to_end(Data_node A, Data_node B);
double compute_weight_road(Data_road* road);
void fill_path_node(std::vector<Data_node>& vector_node, std::vector<Data_road>& road_vector, std::vector<Path_node>& graph);
