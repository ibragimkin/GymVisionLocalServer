# Клонируем репозиторий
git clone https://github.com/ibragimkin/GymVisionLocalServer.git
cd GymVisionLocalServer

# Создаём build-директорию и собираем проект
mkdir build && cd build

cmake .. && make

# Запускаем сервер
./GymVisionLocalServer

cd ../py && uvicorn bridge:app --host 0.0.0.0 --port 8000

# Использование интерфейса коммандной строки:
./camera_cli [subbcomand]

Subcommands:

  add-camera <id> <name> <ip> <port> <username> <password> <video_url>                 Add a camera to the database
  
  start-stream    <id>            Start streaming from a camera
  
  start-stream-cv     <id>        Start streaming with computer vision overlay
  
  check-camera   <id>             Check if a camera exists
  
  move-camera   <id>              Move camera PTZ by x and y for time
  
