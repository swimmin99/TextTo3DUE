@echo off
setlocal

:: ==============================================================================
:: Shap-E Plugin Execution Script
::
:: This script serves two purposes:
:: 1. Configuration Mode: When run directly by a user, it prompts for the
::    Anaconda path and environment name, saving them to a config file.
:: 2. Unreal Mode: When called by Unreal Engine with the "--ue" argument,
::    it reads the config, activates the specified Conda environment, and
::    executes the Python interface script, passing along all arguments.
:: ==============================================================================

:: Define paths relative to the script's location.
set "CONFIG_FILE=%~dp0shape_config.txt"
set "PYTHON_SCRIPT_PATH=%~dp0ue_shape_interface.py"

:: --- Mode Determination ---
:: If the first argument is "--ue", switch to automated execution mode.
:: Otherwise, enter interactive setup mode for the user.
if /i "%~1"=="--ue" (
    goto :UNREAL_MODE
) else (
    goto :MANUAL_SETUP
)


:: ##############################################################################
:MANUAL_SETUP
:: # Interactive configuration mode for first-time user setup.
:: ##############################################################################
title Shap-E Plugin Setup
cls
echo =================================================
echo  Shap-E Plugin - First-Time Setup
echo =================================================
echo.
echo This utility will configure the Anaconda environment
echo needed for the plugin to run.
echo.
echo -------------------------------------------------
echo.
if exist "%CONFIG_FILE%" (
    echo An existing configuration was found:
    echo.
    for /f "usebackq tokens=1,* delims==" %%A in ("%CONFIG_FILE%") do (
        set "%%A=%%B"
        echo   [%%A] = %%B
    )
    echo.
    echo -------------------------------------------------
    echo.
    set "RECONFIGURE="
    set /p "RECONFIGURE=Do you want to re-configure? (y/n) [n]: "
    if /i not "%RECONFIGURE%"=="y" goto :END_MANUAL
    echo.
)
:GET_NEW_CONFIG
echo Please provide the required paths.
echo.
:GET_ANACONDA_PATH
set "ANACONDA_BASE_INPUT="
echo [Example] C:\Users\YourName\anaconda3
set /p "ANACONDA_BASE_INPUT=Enter the FULL PATH to your Anaconda installation: "
if "%ANACONDA_BASE_INPUT%"=="" ( echo. & echo ERROR: Path cannot be empty. & echo. & goto :GET_ANACONDA_PATH )
if not exist "%ANACONDA_BASE_INPUT%\Scripts\conda.exe" (
    echo. & echo ERROR: 'conda.exe' not found in the specified path. Please verify. & echo. & goto :GET_ANACONDA_PATH
)
set "ANACONDA_BASE=%ANACONDA_BASE_INPUT%"

:GET_ENV_NAME
echo.
set "CONDA_ENV_NAME_INPUT="
echo [Example] shap-e
set /p "CONDA_ENV_NAME_INPUT=Enter the NAME of your Conda environment: "
if "%CONDA_ENV_NAME_INPUT%"=="" ( echo. & echo ERROR: Environment name cannot be empty. & echo. & goto :GET_ENV_NAME )
set "CONDA_ENV_NAME=%CONDA_ENV_NAME_INPUT%"

:: Write the new configuration to the file.
(
    echo ANACONDA_BASE=%ANACONDA_BASE%
    echo CONDA_ENV_NAME=%CONDA_ENV_NAME%
) > "%CONFIG_FILE%"

echo.
echo =================================================
echo  Configuration saved successfully!
echo =================================================
echo.
:END_MANUAL
echo You can now use the plugin from within Unreal Engine.
echo This window can be closed.
echo.
pause
exit /b 0


:: ##############################################################################
:UNREAL_MODE
:: # Automated execution mode, called by the Unreal Engine process.
:: ##############################################################################

:: Check for the configuration file. If not found, report error to UE.
if not exist "%CONFIG_FILE%" (
    echo {"type":"error", "error_type":"ConfigurationMissing", "message":"Configuration file 'shape_config.txt' not found. Please run 'run_shape.bat' once manually to set it up."}
    exit /b 1
)

:: Load settings from the configuration file.
for /f "usebackq tokens=1,* delims==" %%A in ("%CONFIG_FILE%") do (
    set "%%A=%%B"
)

:: Validate that the required variables were loaded.
if not defined ANACONDA_BASE (
    echo {"type":"error", "error_type":"ConfigurationInvalid", "message":"ANACONDA_BASE is not defined in the config file."}
    exit /b 1
)
if not defined CONDA_ENV_NAME (
    echo {"type":"error", "error_type":"ConfigurationInvalid", "message":"CONDA_ENV_NAME is not defined in the config file."}
    exit /b 1
)

:: Activate the Conda environment using the official activation script.
call "%ANACONDA_BASE%\Scripts\activate.bat" %CONDA_ENV_NAME%
if %errorlevel% neq 0 (
    echo {"type":"error", "error_type":"EnvActivationFailed", "message":"Failed to activate conda environment '%CONDA_ENV_NAME%'. Please verify settings in the config file."}
    exit /b 1
)

:: --- Core Execution ---
:: With the environment active, run the Python script.
:: The -u flag ensures unbuffered output, allowing UE to read it in real-time.
:: %* forwards all arguments passed to this batch script (e.g., --ue, --params-base64)
:: directly to the Python script.
python -u "%PYTHON_SCRIPT_PATH%" %*