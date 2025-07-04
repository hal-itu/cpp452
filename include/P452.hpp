#define _USE_MATH_DEFINES
#ifndef PI
#define PI 3.14159265358979323846
#endif

#include <math.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <cassert>

#pragma once

using namespace std;

// Define classes for dealing with P452 digital maps DN50.TXT and N050.TXT

class DigitalMap {
  std::vector<std::vector<double> > _map;
  size_t _sizeX, _sizeY;

 private:
  double _spacing;

 public:
  DigitalMap(std::string path, bool lastAndFirstColumnEqual = true) {
    std::ifstream in(path);

    // check that the path can be loaded
    try {
      if (in.fail()) {
        std::cerr << "File " << path << " cannot be opened." << std::endl;
        throw;
      }
    } catch (std::exception& e) {
      std::cerr << "exception: " << e.what() << std::endl;
    }

    std::string record;

    while (std::getline(in, record)) {
      std::istringstream is(record);
      std::vector<double> row((std::istream_iterator<double>(is)),
                              std::istream_iterator<double>());
      _map.push_back(row);
    }

    _sizeX = _map[0].size();  // ncols
    _sizeY = _map.size();     // nrows

    if (lastAndFirstColumnEqual) {
      _spacing = 360.0 / (_map[1].size() - 1);
    } else {
      _spacing = 360.0 / (_map[1].size());
    }
  }

 public:
  DigitalMap(void)
  // default constructor
  {
    _sizeX = 0;  // ncols
    _sizeY = 0;  // nrows
    _spacing = 0;
  }

 private:
  double GetClosestGridPointValue(double longitude, double latitude) {
    double longitudeOffset = longitude + 180.0;
    double latitudeOffset = 90.0 - latitude;
    int latitudeIndex = (int)(latitudeOffset / _spacing);
    int longitudeIndex = (int)(longitudeOffset / _spacing);
    latitudeIndex %= _sizeY;
    longitudeIndex %= _sizeX;

    double val = _map[latitudeIndex][longitudeIndex];
    return val;
  }

 public:
  double GetInterpolatedValue(double longitude, double latitude) {
    double longitudeOffset = longitude;
    if (longitude < 0.0) {
      longitudeOffset = longitude + 360.0;
    }
    double latitudeOffset = 90.0 - latitude;
    int latitudeIndex = (int)(latitudeOffset / _spacing);
    int longitudeIndex = (int)(longitudeOffset / _spacing);
    latitudeIndex %= _sizeY;
    longitudeIndex %= _sizeX;

    double latitudeFraction = (latitudeOffset / _spacing) - latitudeIndex;
    double longitudeFraction = (longitudeOffset / _spacing) - longitudeIndex;

    assert(longitudeIndex >= 0);
    assert(longitudeIndex + 1 < _sizeX); //todo: fix map access for longitude = 360 degree

    double value_ul = _map[latitudeIndex][longitudeIndex];    
    double value_ur = _map[latitudeIndex][(longitudeIndex + 1) % _sizeX];
    double value_ll = _map[(latitudeIndex + 1) % _sizeY][longitudeIndex];
    double value_lr =
        _map[(latitudeIndex + 1) % _sizeY][(longitudeIndex + 1) % _sizeX];

    double interpolatedHeight1 =
        (longitudeFraction * (value_ur - value_ul)) + value_ul;
    double interpolatedHeight2 =
        (longitudeFraction * (value_lr - value_ll)) + value_ll;
    double interpolatedHeight3 =
        latitudeFraction * (interpolatedHeight2 - interpolatedHeight1) +
        interpolatedHeight1;

    return interpolatedHeight3;
  }
};

class P452DigitalMaps {
 private:
  DigitalMap _mapDN50;

 private:
  DigitalMap _mapN050;

 public:
  P452DigitalMaps(std::string directory) {
    _mapDN50 = DigitalMap(directory + "DN50.TXT");
    _mapN050 = DigitalMap(directory + "N050.TXT");
  }

 public:
  double GetDN50(double lon, double lat) {
    return _mapDN50.GetInterpolatedValue(lon, lat);
  }

 public:
  double GetN050(double lon, double lat) {
    return _mapN050.GetInterpolatedValue(lon, lat);
  }
};

double tl_p452(P452DigitalMaps maps,
               double f,
               double p,
               std::vector<double>& d,
               std::vector<double>& h,
               std::vector<double>& g,
               std::vector<int>& zone,
               double htg,
               double hrg,
               double phit_e, 
               double phit_n, 
               double phir_e, 
               double phir_n,
               double Gt,
               double Gr,
               int pol,
               double dct,
               double dcr,
               double press,
               double temp);

void great_circle_path(double Phire,
                       double Phite,
                       double Phirn,
                       double Phitn,
                       double Re,
                       double dpnt,
                       double& Phipnte,
                       double& Phipntn,
                       double& Bt2r,
                       double& dgc);
