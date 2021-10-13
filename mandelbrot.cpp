#include <iostream>
#include <cmath>
#include <complex>
#include <tuple>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <vector>

const int MAX_ITER = 50;
const int THREAD_NUM = 8;


//rozhodne jestli k cislo do Mandelbrota patri a pokud ne tak u jakeho n zacne byt vetsi nez 2:
std::tuple<bool, int> Mandelbrot(std::complex<double> c){ 
    std::complex<double> current(0.0, 0.0); 

    for(auto i = 1; i < MAX_ITER; i ++){
        if(abs(current) > 2)
            return std::make_tuple(false, i-1);
        current = std::pow(current,2) + c;
    }
    return std::make_tuple(true, MAX_ITER); 
}

struct Color{
    unsigned char R, G, B;
    //3 helpful constructors
    Color(unsigned char r, unsigned char g, unsigned char b){
        R = r;
        G = g;
        B = b;
    }
    Color(unsigned char c) : Color(c, c, c) {};
    Color() : Color(0) {};
};

class Image {
    Color* data;
    size_t height, width;
    public:
    Image(size_t w, size_t h, Color c){
        height = h;
        width = w;
        data = new Color[w*h];
    }
    Image(size_t h, size_t w) : Image(h, w, Color(0)) {};
    ~Image(){
        delete[] data;
    }
    size_t get_width(){return width;}
    size_t get_height(){return height;}
    Color& operator() (size_t x, size_t y) {
        if (x < width && y < height) {return data[y*width + x];} 
        throw std::out_of_range("X: " + std::to_string(x) + " Y: " + std::to_string(y));
    }
    bool save(std::string path){
        std::ofstream output(path, std::ios::out | std::ios::trunc | std::ios::binary);
        if(output.is_open()){
            output << "P6" << std::endl;
            output << width << ' ' << height << std::endl;
            output << "255" << std::endl;
            output.write((char*)data, width*height*3);
            output.close();
            return true;
        }
        else{
            return false;
        }
    }
};


void make_image(Image *imag, std::array<Color, 16> *array_col, double start_r, double stop_r, double start_c, double stop_c, double thrd_start_c, double thrd_stop_c){
    double height = imag->get_height();
    double width =  imag->get_width();

    double len_r = fabs(start_r - stop_r);
    double step_r = len_r / width;
    double len_c = fabs(start_c - stop_c);
    double step_c = len_c / height;
    
    std::complex<double> c;
    std::tuple<bool,int> patri;
    for(auto x = 0; x < width; x++){
        for(auto y = (thrd_start_c-start_c)*(height/len_c); y < (thrd_stop_c-start_c)*(height/len_c); y ++){
            c.imag(((len_c*y)/height)+start_c);
            c.real(((len_r*x)/width)+start_r);
            patri = Mandelbrot(c);
            if(std::get<0>(patri)){
                (*imag)(x, y) = Color(255);
            }
            else{
                (*imag)(x, y) = (*array_col)[std::get<1>(patri) % 16];
            }
        }
    }
}

int main(){

    const double start_r = -2.3, stop_r = 1.3;
    const uint width = 3600;
    const double start_c = -1.4 ,stop_c = 1.2;
    const uint height = 2600;

    Image imag(width, height);

    std::array<Color,16> array_col = {
        Color(66, 30, 15), Color(25, 7, 26), Color(9,1,47), Color(4, 4, 73),
        Color(0, 7, 100), Color(12, 44, 138), Color(24, 82, 177), Color(57, 125, 209),
        Color(134, 181, 229), Color(211, 236, 248), Color(241, 233, 191), Color(248, 201, 95),
        Color(255, 170, 0), Color(204, 128, 0), Color(153, 87, 0), Color((106, 52, 3))
    };
    
    //multithreaded aby to rychleji pocitalo:
    std::thread threads[THREAD_NUM];
    double len_c = fabs(start_c - stop_c);
    for(auto i = 0; i < THREAD_NUM; i ++){
        std::cout << i << std::endl;
        threads[i] = std::thread(&make_image, &imag, &array_col, start_r, stop_r, start_c, stop_c,
            i*(len_c/THREAD_NUM) + start_c, (i+1)*(len_c/THREAD_NUM) + start_c);
    }
    for(auto i = 0; i < THREAD_NUM; i ++){
        threads[i].join(); //aby se cekalo nez dopocitaji vsechny
    }
    
    imag.save("mandelbrot3.ppm");
}

//approach desetinnyma cislama, problemy s nepresnosti
/* for(double r = start_r; r < stop_r; r+=step_r){ 
        for(double i = _start_c; i < _stop_c; i+=step_c){
            c.imag(i);
            c.real(r);
            patri = Mandelbrot(c);
            int y = (i-start_c)*(height/len_c);
            int x = (r-start_r)*(width/len_r);
            if(std::get<0>(patri)){
                (*imag)(x, y) = Color(255);
            }
            else{
                (*imag)(x, y) = (*array_col)[std::get<1>(patri) % 16];
            }
        }
    } */
