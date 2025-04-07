## NetCDF Server

<p> NetCDF server for a 4D concentration series based on concentration.timeseries.nc from Aeris LLC. This is meant to be run completely within a Docker container.
    <br> 
</p>

## Table of Contents

- [About](#about)
- [Getting Started](#getting_started)
- [Tests](#tests)
- [Deployment](#deployment)
- [Built Using](#built_using)
- [Authors](#authors)

## About <a name = "about"></a>

1. Publicly hosted on Github<br>
2. The use of Crow C++ REST framework<br>
3. The following endpoints are implementedImplementation of the following endpoints<br>
a. /get-info, returns the NetCDF detailed information.<br>
b. /get-data, params to include time index and z index, <br>
returns json response that includes x, y, and concentration data.<br>
c. /get-image, params to include time index and z index, <br>
returns png visualization of concentration.<br>
4. Dockerfile for container deployment<br>
5. This README.md

## Getting Started <a name = "getting_started"></a>

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See [deployment](#deployment) for notes on how to deploy the project on a live system.

### Pre-requisites

Docker runtime environment.

```
brew install docker - or your platform specific installer (apt, yum, etc)
```

```
Visit <a href="https://www.docker.com">Docker</a> for more information and installation options.
```

### Installing

Download and unzip this project structure into a directory on your local machine, or open in GitHub desktop:

```
Click green code dropdown and choose either the Download ZIP or Open option.
```

Clone repository locally: 

```
git clone https://github.com/jodelcharles/netCDF-server.git
```

Navigate to project root on your local machine (verify Dockerfile is present) and build:

```
docker build --no-cache -t netcdf-server . --progress=plain
```

The project <a href="Dockerfile">Dockerfile</a> gets all the necessary executables and libraries needed to build and run the server under /app.
This includes copying the necessary headers from the include folder into the container.

<a href="CMakeLists.txt">CMakeLists.txt</a> utilizes that and points to include and link directories appropriately.

```
include_directories(/app/lib/include /app/include)
link_directories(/app/lib)
```

## Tests <a name = "tests"></a>

TO-DO: Add automated tests for unit testing:

```
TEST_CASE("NetCDF file opens") {
    REQUIRE_NOTHROW(NetCDFServer("data/concentration.timeseries.nc"));
    REQUIRE_THROWS(NetCDFServer("data/concentration.timeseries.ncbad"));
}
```

```
Test extractNetCDFSlice() for returns of the correct values for given valid time and z indices.
```

```
Test generateVisual() for proper PNG file creation given valid time and z indices.
```

Use curl, libcurl, etc for integration testing:

```
curl "http://localhost:18080/get-data?time=0&z=0" | jq .
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 14178  100 14178    0     0   697k      0 --:--:-- --:--:-- --:--:--  728k
{
  "x": [
    0,
    285.714285714285722406202694401144981,
    571.428571428571444812405388802289963,
    857.142857142857110375189222395420074,
    1142.85714285714288962481077760457993,
    1428.57142857142866887443233281373978,
    1714.28571428571422075037844479084015,
    2000, truncated
```

``` 
curl "http://localhost:18080/get-info"
```

```
curl "http://localhost:18080/get-image?time=1&z=0"
```

Edge cases examples
```
curl "http://localhost:18080/get-data?time=0&z=2" | jq .
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100    57  100    57    0     0  17096      0 --:--:-- --:--:-- --:--:-- 19000
{
  "error": "z index out of range - Cannot exceed 0."
}
```

```
curl "http://localhost:18080/get-data?time=0" | jq .  
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100    58  100    58    0     0  12169      0 --:--:-- --:--:-- --:--:-- 14500
{
  "error": "Missing required parameters: time and z."
}
jodelcharles@Js-MacBook-Pro concentration_netcdf % 
```

## Deployment <a name = "deployment"></a>

Run the docker container (the server runs on localhost port 18080):

```
docker run --rm -p 18080:18080 netcdf-server
```

Open your browser and test the /get-info endpoint:

```
<a href=http://localhost:18080/get-info>http://localhost:18080/get-info</a>
```

## Built Using <a name = "built_using"></a>

- [CrowCPP](https://crowcpp.org/master/) - C++ REST Framework
- [Matplot++](https://alandefreitas.github.io/matplotplusplus/) - PNG Visualization
- [NetCDF CXX4](https://unidata.github.io/netcdf-cxx4) - NetCDF API
- [Visual Studio Code](https://code.visualstudio.com) - IDE
- [Docker](https://www.docker.com) - Docker

## Authors <a name = "authors"></a>

- [@jodelcharles](https://github.com/jodelcharles)