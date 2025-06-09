# TextTo3DUE
Request and Receive 3d model from text prompt using shap e in UE environment
TextTo3DUE는 OpenAI의 3D 모델 생성 AI인 Shap-E를 Unreal Engine 에디터 내에서 직접 사용할 수 있도록 만든 비공식 플러그인입니다. 텍스트 프롬프트만으로 3D 메시를 생성하고, 생성된 모델을 즉시 프로젝트에 활용해 보세요.

## 주요 기능
에디터 내 3D 모델 생성: Unreal Engine 에디터를 벗어날 필요 없이 텍스트 프롬프트로 3D 모델을 생성합니다.
실시간 진행률 확인: 모델 생성 과정을 실시간 프로그레스 바와 상태 메시지로 확인할 수 있습니다.
간편한 파라미터 조정: Guidance Scale, Karras Steps 등 주요 생성 파라미터를 UI에서 쉽게 조절할 수 있습니다.
자동 파일 저장: 생성된 모델은 지정된 폴더에 .ply와 .obj 형식으로 자동 저장됩니다.

## 설치 및 요구사항
이 플러그인을 사용하기 위해서는 로컬 컴퓨터에 Python 환경과 필요한 라이브러리들이 먼저 설치되어 있어야 합니다.

요구사항:
Unreal Engine 5.6
Anaconda 또는 Miniconda
NVIDIA GPU (CUDA 지원)

### 1단계: 로컬 모델 설치
shap e를 로컬 환경에서 구동하도록 설치해야 합니다.
local 버전을 쉽게 구현하기 위해서 다음의 리포지토리를 추천합니다.  
https://github.com/kedzkiest/shap-e-local

로컬 모델 설치 관련 사항은 위 리포지토리의 README를 참조하시기 바랍니다.

### 2단계: 플러그인 설치
이 GitHub 저장소의 Releases 페이지에서 최신 버전의 플러그인 파일을 다운로드합니다.
압축을 해제합니다.
플러그인 폴더 (ShapEForUE 등)를 Unreal Engine 프로젝트의 Plugins 폴더로 복사합니다.
예: C:/Users/YourName/Documents/Unreal Projects/MyProject/Plugins/ShapEForUE
Plugins 폴더가 없다면 새로 만드세요.
프로젝트를 다시 시작하고, Edit > Plugins 메뉴에서 플러그인이 활성화되어 있는지 확인합니다.

### 3단계: 모델에 UE측 접속 인터페이스 삽입하기

Python 환경과 플러그인을 연결해야 합니다.
플러그인 폴더 내에 있는 run_shape.bat 파일과 ue_shape_interface.py 파일을 찾습니다.
경로: YourProject/Plugins/TextTo3DRequest/Resources
run_shape.bat 파일과 ue_shape_interface.py 파일을 로컬 shape 모델의 최상위 루트로 옮깁니다.
run_shape.bat 파일을 더블 클릭하여 실행합니다.
설정 창이 나타나면, 1단계에서 설정한 Anaconda의 정보를 입력합니다.
Anaconda Base Path: Anaconda가 설치된 기본 경로 (예: C:\Users\YourName\anaconda3)
Conda Environment Name: 1단계에서 생성한 가상 환경의 이름 (예: shape-env)
설정이 완료되면 창을 닫습니다. 이제 플러그인을 사용할 준비가 되었습니다.

### 4단계: 플러그인 사용 방법
Unreal Engine 에디터에서 Tools > Shap-E Generator 메뉴를 선택하여 플러그인 창을 엽니다.
Batch File: run_shape.bat 파일의 경로가 올바르게 지정되었는지 확인합니다. (기본 경로를 참고하여 Browe)
Prompt: 생성하고 싶은 3D 모델에 대한 설명을 영어로 입력합니다. (예: a red sports car, a wizard's hat)
Output Directory: 생성된 모델 파일(.ply, .obj)이 저장될 프로젝트 내 경로를 지정합니다.
Parameters: Guidance Scale, Karras Steps 등 세부 파라미터를 조절하여 결과물의 퀄리티를 조정합니다.
Generate Model 버튼을 클릭하여 생성을 시작합니다.
생성 중에는 Cancel 버튼으로 작업을 중단할 수 있습니다.
생성이 완료되면 로그 창에 파일 경로가 표시되고, Done 버튼을 눌러 상태를 초기화할 수 있습니다.
이후, UE 에디터가 자동으로 Import할 것인지를 묻는 창을 띄웁니다. Import하기를 누릅니다.


# TextTo3DUE

Request and Receive 3D Models from Text Prompts Using Shap-E in UE Environment  
TextTo3DUE is an unofficial plugin that lets you use OpenAI’s 3D model generator Shap-E directly in the Unreal Engine Editor. Generate 3D meshes from text prompts and use them in your project instantly.

## Key Features

- **3D Model Generation Inside the Editor**: Create 3D models from text prompts without leaving the Unreal Engine Editor.
- **Live Progress Tracking**: Watch model generation in real time with a progress bar and status messages.
- **Easy Parameter Adjustment**: Tweak generation parameters like Guidance Scale and Karras Steps directly in the UI.
- **Automatic File Saving**: Generated models are saved automatically in .ply and .obj formats in your chosen folder.

## Installation & Requirements

You need a local Python environment and required libraries to use this plugin.

**Requirements:**
- Unreal Engine 5.6
- Anaconda or Miniconda
- NVIDIA GPU (CUDA support)

### Step 1: Install the Local Shap-E Model

You need to run Shap-E locally.  
For easy setup, use: https://github.com/kedzkiest/shap-e-local  
See that repo’s README for instructions.

### Step 2: Install the Plugin

- Download the latest plugin from this GitHub repo’s Releases page.
- Extract it.
- Copy the plugin folder (e.g., ShapEForUE) into your Unreal project’s Plugins directory.  
  Example: `C:/Users/YourName/Documents/Unreal Projects/MyProject/Plugins/ShapEForUE`
  (Create a Plugins folder if it doesn’t exist.)
- Restart your project and check Edit > Plugins to enable it.

### Step 3: Connect UE Plugin to Local Model

- Find `run_shape.bat` and `ue_shape_interface.py` inside `YourProject/Plugins/TextTo3DRequest/Resources`.
- Move both files to the root folder of your local Shap-E model.
- Double-click `run_shape.bat`.
- Enter the following in the setup window:
    - **Anaconda Base Path**: e.g., `C:\Users\YourName\anaconda3`
    - **Conda Environment Name**: The environment name from Step 1 (e.g., `shape-env`)
- Close the setup window when done. You’re ready to go!

### Step 4: How to Use

- In Unreal Engine, open **Tools > Shap-E Generator**.
- Make sure Batch File points to `run_shape.bat`.
- Enter your model description in English in Prompt (e.g., `a red sports car`).
- Set Output Directory to your project folder for saving `.ply` and `.obj`.
- Adjust Parameters like Guidance Scale, Karras Steps, etc.
- Click **Generate Model**.
- Cancel any time with the Cancel button.
- When complete, file paths show in the log. Click Done to reset.
- UE will prompt to import the new model. Click Import.

---


## 프로젝트 세부 구조
UE 플러그인 (C++ & Slate):
SShapEGenerationWidget: 사용자가 프롬프트와 파라미터를 입력하고, 생성 과정을 시각적으로 확인할 수 있는 UI입니다. Unreal Engine의 UI 프레임워크인 Slate를 사용하여 제작되었습니다.

FShapEProcessManager: 외부 Python 프로세스의 생성, 관리, 통신을 총괄하는 핵심 매니저 클래스입니다. FRunnable 을 사용하여 Python 스크립트의 출력을 비동기적으로 읽어와 에디터의 응답성을 유지합니다.

런처 스크립트 (run_shape.bat):
C++와 Python 사이의 중간 다리 역할을 합니다.
설정 모드: 사용자가 직접 실행하면 Anaconda 경로와 가상 환경 이름을 설정하여 shape_config.txt 파일에 저장합니다.
실행 모드: Unreal Engine에서 --ue 인자와 함께 호출되면, 설정된 Conda 환경을 자동으로 활성화하고 Python 스크립트를 실행합니다.
Python 인터페이스 (ue_shape_interface.py):
Unreal Engine으로부터 파라미터를 받아 Shap-E 모델을 실행하는 워커 스크립트입니다.
argparse를 사용하여 명령줄 인자로 전달된 생성 파라미터를 안전하게 파싱합니다.
모델 생성 과정의 상태(status), 진행률(progress), 완료(complete), 에러(error)를 JSON 형식으로 표준 출력(stdout)에 기록하여 Unreal Engine과 통신합니다.
통신 방식 (IPC)
UE → Python (파라미터 전달):
FPlatformProcess::CreateProc 함수를 사용하여 .bat 파일을 실행합니다.
사용자가 입력한 모든 파라미터(프롬프트, 스텝 등)는 하나의 JSON 문자열로 직렬화된 후, Base64로 인코딩되어 안전하게 명령줄 인자로 전달됩니다.
Python → UE (결과 및 로그 전달):
Python 스크립트는 모든 출력을 stdout으로 보냅니다.
C++의 FShapEOutputReaderRunnable를 통해 이 출력을 실시간으로 읽습니다.
수신된 데이터는 게임 스레드로 전달 된 후, 파싱되어 UI에 반영되거나 로그로 기록됩니다. 
tqdm 라이브러리가 출력하는 비-JSON 형식의 진행률 텍스트도 C++에서 직접 파싱하여 UI 프로그레스 바에 반영합니다.

## Project Structure

### UE Plugin (C++ & Slate):

- **SShapEGenerationWidget**:  
  The main UI for prompt/parameter entry and real-time progress/status using Slate.
- **FShapEProcessManager**:  
  Manages external Python process, handles all communication and monitoring.
  Uses FRunnable to asynchronously read Python output, keeping UE responsive.

### Launcher Script (`run_shape.bat`):

- Bridge between C++ and Python.
- **Setup Mode**:  
  When run directly, configures Anaconda path and environment, saving to `shape_config.txt`.
- **Execution Mode**:  
  When called by UE with `--ue`, activates the Conda environment and runs the Python script.

### Python Interface (`ue_shape_interface.py`):

- Receives parameters from UE and runs the Shap-E model.
- Uses argparse for safe CLI parsing.
- Outputs status, progress, completion, and errors as JSON via stdout for UE to read.

- **UE → Python (parameter passing)**:  
  UE uses FPlatformProcess::CreateProc to run the batch file.
  All parameters (prompt, steps, etc.) are serialized as a JSON string, Base64-encoded, and passed as a command-line argument—avoiding complicated pipes and deadlocks.

- **Python → UE (results/logs)**:  
  Python outputs all info via stdout.
  FShapEOutputReaderRunnable (background thread) reads output asynchronously, sends updates to the game thread for safe UI/logging.
  Non-JSON status (like tqdm progress) is also parsed for the UE progress bar, keeping the Editor responsive.

---










