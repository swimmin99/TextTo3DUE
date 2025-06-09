#
# This script generates a 3D mesh from a text prompt using OpenAI's Shap-E model.
# It is designed to be called from an external application (Unreal Engine),
# receiving parameters via a Base64-encoded JSON string and sending progress
# and results back via JSON messages printed to stdout.
#

import sys
import json
import os
import traceback
import torch
import argparse
import base64
import warnings

# Suppress a specific FutureWarning from a dependency.
warnings.filterwarnings("ignore", category=FutureWarning)

from shap_e.diffusion.sample import sample_latents
from shap_e.diffusion.gaussian_diffusion import diffusion_from_config
from shap_e.models.download import load_model, load_config
from shap_e.util.notebooks import decode_latent_mesh

def send_json_message(data):
    """Sends a JSON-formatted message to stdout for the calling process."""
    try:
        json_string = json.dumps(data, separators=(',', ':'))
        print(json_string, flush=True)
    except Exception as e:
        # Failsafe in case the data itself can't be serialized.
        error_msg = {"type": "internal_error", "message": f"send_json_message failed: {str(e)}"}
        print(json.dumps(error_msg), flush=True)

def run_generation(params):
    """Handles the core logic of model loading, generation, and file saving."""
    prompt = params.get("prompt")
    output_dir = params.get("output_dir")
    guidance_scale = float(params.get("guidance_scale", 15.0))
    karras_steps = int(params.get("karras_steps", 64))
    use_fp16 = bool(params.get("use_fp16", True))

    os.makedirs(output_dir, exist_ok=True)
    
    send_json_message({"type": "status", "message": "Setting up device..."})
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    if not torch.cuda.is_available():
        raise RuntimeError("CUDA is not available. This script requires a GPU.")
    send_json_message({"type": "status", "message": f"Device set to: {device}"})

    send_json_message({"type": "status", "message": "Loading models..."})
    xm = load_model('transmitter', device=device)
    model = load_model('text300M', device=device)
    diffusion = diffusion_from_config(load_config('diffusion'))
    send_json_message({"type": "status", "message": "Models loaded."})

    send_json_message({"type": "status", "message": f"Generating latents for prompt: '{prompt}'..."})
    latents = sample_latents(
        batch_size=1,
        model=model,
        diffusion=diffusion,
        guidance_scale=guidance_scale,
        model_kwargs=dict(texts=[prompt]),
        progress=True,
        clip_denoised=True,
        use_fp16=use_fp16,
        use_karras=True,
        karras_steps=karras_steps,
        sigma_min=1e-3,
        sigma_max=160,
        s_churn=0,
    )
    
    send_json_message({"type": "status", "message": "Latents generation complete. Decoding to mesh..."})
    
    decoded_output = decode_latent_mesh(xm, latents[0])
    
    # The raw decoded output must be converted to a TriMesh object to be saved.
    final_mesh_to_save = decoded_output.tri_mesh()

    # Sanitize the prompt to create a safe filename.
    safe_prompt_portion = "".join(c if c.isalnum() or c in (' ', '_') else '_' for c in prompt[:50]).rstrip()
    mesh_filename_base = "_".join(safe_prompt_portion.split()).lower() or "generated_model"

    ply_filepath = os.path.join(output_dir, f'{mesh_filename_base}.ply')
    obj_filepath = os.path.join(output_dir, f'{mesh_filename_base}.obj')

    # Save the mesh in both PLY and OBJ formats.
    try:
        with open(ply_filepath, 'wb') as f:
            final_mesh_to_save.write_ply(f)
        send_json_message({"type": "status", "message": f"Saved PLY to: {ply_filepath}"})
    except Exception as e:
        send_json_message({"type": "error", "message": f"Failed to save PLY file: {str(e)}"})
        ply_filepath = None

    try:
        with open(obj_filepath, 'w', encoding='utf-8') as f:
            final_mesh_to_save.write_obj(f)
        send_json_message({"type": "status", "message": f"Saved OBJ to: {obj_filepath}"})
    except Exception as e:
        send_json_message({"type": "error", "message": f"Failed to save OBJ file: {str(e)}"})
        obj_filepath = None

    send_json_message({
        "type": "complete",
        "message": "Generation Complete! Files saved.",
        "ply_file": os.path.abspath(ply_filepath) if ply_filepath and os.path.exists(ply_filepath) else None,
        "obj_file": os.path.abspath(obj_filepath) if obj_filepath and os.path.exists(obj_filepath) else None
    })

def main():
    """Parses command-line arguments and initiates the generation process."""
    try:
        send_json_message({ "type": "info", "message": "Python script started." })
        
        parser = argparse.ArgumentParser()
        parser.add_argument("--ue", action="store_true", help="Flag for Unreal Engine specific behavior (if any).")
        parser.add_argument("--params-base64", type=str, required=True, help="Base64 encoded JSON string of parameters.")
        args = parser.parse_args(sys.argv[1:])
        
        # Decode the parameters from the command line argument.
        json_string = base64.urlsafe_b64decode(args.params_base64).decode('utf-8')
        params = json.loads(json_string)
        
        run_generation(params)
    except Exception as e:
        # Catch any critical error and report it back to the calling process.
        send_json_message({
            "type": "error",
            "message": f"A critical error occurred: {str(e)}",
            "error_type": e.__class__.__name__,
            "traceback": traceback.format_exc()
        })

if __name__ == '__main__':
    main()