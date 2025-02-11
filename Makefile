INCLUDES=-Iinclude 
LIBS= libraylib.a -lopengl32 -lgdi32 -lwinmm -I include

BASE_FLAGS=-DGLM_FORCE_LEFT_HANDED
DEBUG_FLAGS=-DEDITOR_DEBUG -g
ALLOBJECTS=build/obj/main.o build/obj/shader_store.o build/obj/AABB.o build/obj/pathfinding.o  build/obj/raylib_extensions.o build/obj/tile_map.o  build/obj/mesh_utils.o build/obj/particle_system.o build/obj/camera.o build/obj/g_main.o build/obj/game_state.o build/obj/state_main.o

all: executable
debug: BASE_FLAGS += $(DEBUG_FLAGS)
debug: executable

run: 
	./build/main.exe

executable: $(ALLOBJECTS)
	g++ $(BASE_FLAGS) $(ALLOBJECTS) -o build/main $(INCLUDES) $(LIBS)

build/obj/main.o: main.cpp
	g++  $(BASE_FLAGS) -c main.cpp -o build/obj/main.o $(INCLUDES) 

build/obj/mesh_utils.o: mesh_utils.cpp
	g++  $(BASE_FLAGS) -c mesh_utils.cpp -o build/obj/mesh_utils.o $(INCLUDES) 

build/obj/game_state.o: game_state.cpp game_state.hpp
	g++  $(BASE_FLAGS) -c game_state.cpp -o build/obj/game_state.o $(INCLUDES) 

build/obj/AABB.o: AABB.cpp AABB.hpp
	g++  $(BASE_FLAGS) -c AABB.cpp -o build/obj/AABB.o $(INCLUDES) 

build/obj/particle_system.o: particle_system.cpp particle_system.hpp
	g++  $(BASE_FLAGS) -c particle_system.cpp -o build/obj/particle_system.o $(INCLUDES) 

build/obj/camera.o: camera.cpp camera.hpp
	g++  $(BASE_FLAGS) -c camera.cpp -o build/obj/camera.o $(INCLUDES) 

build/obj/g_main.o: g_main.cpp g_main.hpp
	g++  $(BASE_FLAGS) -c g_main.cpp -o build/obj/g_main.o $(INCLUDES) 

build/obj/shader_store.o: shader_store.cpp shader_store.hpp
	g++  $(BASE_FLAGS) -c shader_store.cpp -o build/obj/shader_store.o $(INCLUDES) 

build/obj/raylib_extensions.o: raylib_extensions.cpp raylib_extensions.hpp
	g++  $(BASE_FLAGS) -c raylib_extensions.cpp -o build/obj/raylib_extensions.o $(INCLUDES) 


## actual game code here

build/obj/state_main.o: game_src/state_main.cpp game_src/state_main.hpp
	g++  $(BASE_FLAGS) -c game_src/state_main.cpp -o build/obj/state_main.o $(INCLUDES) 

build/obj/tile_map.o: game_src/tile_map.cpp game_src/tile_map.hpp
	g++  $(BASE_FLAGS) -c game_src/tile_map.cpp -o build/obj/tile_map.o $(INCLUDES) 

build/obj/pathfinding.o: game_src/pathfinding.cpp game_src/pathfinding.hpp
	g++  $(BASE_FLAGS) -c game_src/pathfinding.cpp -o build/obj/pathfinding.o $(INCLUDES) 

clean:
	del build\main.exe
	del /Q /F build\obj\*