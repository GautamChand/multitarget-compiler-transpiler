from flask import Flask, render_template, request, jsonify
import subprocess
import json
import os

app = Flask(__name__)

# Use relative path for compiler executable
COMPILER_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "compiler.exe")
PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))

TARGET_LANGUAGES = ["python", "cpp", "java"]
SOURCE_LANGUAGES = ["Custom", "C-like", "JavaScript-like", "Python-like"]


@app.route("/", methods=["GET", "POST"])
def index():
    result = {
        "output": "",
        "ast": "",
        "ir": "",
        "optimized_ir": "",
        "errors": [],
        "raw": "",
    }
    code = ""
    target = "python"
    source = "Custom"
    compiled = False

    if request.method == "POST":
        code = request.form.get("code", "")
        target = request.form.get("target", "python")
        source = request.form.get("source", "Custom")

        if not code.strip():
            result["errors"] = ["No source code provided."]
            return render_template(
                "index.html",
                result=result,
                code=code,
                target=target,
                source=source,
                compiled=True,
                target_languages=TARGET_LANGUAGES,
                source_languages=SOURCE_LANGUAGES,
            )

        if target not in TARGET_LANGUAGES:
            result["errors"] = [f"Unsupported target language: {target}"]
            return render_template(
                "index.html",
                result=result,
                code=code,
                target=target,
                source=source,
                compiled=True,
                target_languages=TARGET_LANGUAGES,
                source_languages=SOURCE_LANGUAGES,
            )

        try:
            proc = subprocess.Popen(
                [COMPILER_PATH],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=PROJECT_DIR,
            )

            # Protocol: source code, then ###END### delimiter, then target language
            input_data = code + "\n###END###\n" + target + "\n"
            stdout, stderr = proc.communicate(input_data, timeout=30)

            compiled = True

            # Try to parse JSON output from compiler
            if stdout.strip():
                try:
                    parsed = json.loads(stdout.strip())
                    result["output"] = parsed.get("output", "")
                    result["ast"] = parsed.get("ast", "")
                    result["ir"] = parsed.get("ir", "")
                    result["optimized_ir"] = parsed.get("optimized_ir", "")
                    result["errors"] = parsed.get("errors", [])
                    result["raw"] = stdout.strip()
                except json.JSONDecodeError:
                    # If compiler output is not JSON, treat as raw output
                    result["output"] = stdout.strip()
                    result["raw"] = stdout.strip()
            elif stderr.strip():
                result["errors"] = [stderr.strip()]
                result["raw"] = stderr.strip()
            else:
                result["errors"] = ["Compiler produced no output."]

            # Check return code
            if proc.returncode != 0 and not result["errors"]:
                result["errors"] = [
                    f"Compiler exited with code {proc.returncode}."
                ]

        except subprocess.TimeoutExpired:
            result["errors"] = ["Compilation timed out (30s limit)."]
            compiled = True
        except FileNotFoundError:
            result["errors"] = [
                f"Compiler executable not found at: {COMPILER_PATH}"
            ]
            compiled = True
        except Exception as e:
            result["errors"] = [f"Unexpected error: {str(e)}"]
            compiled = True

    return render_template(
        "index.html",
        result=result,
        code=code,
        target=target,
        source=source,
        compiled=compiled,
        target_languages=TARGET_LANGUAGES,
        source_languages=SOURCE_LANGUAGES,
    )


@app.route("/api/compile", methods=["POST"])
def api_compile():
    """AJAX endpoint for compilation without full page reload."""
    data = request.get_json()
    code = data.get("code", "")
    target = data.get("target", "python")

    result = {
        "output": "",
        "ast": "",
        "ir": "",
        "optimized_ir": "",
        "errors": [],
    }

    if not code.strip():
        result["errors"] = ["No source code provided."]
        return jsonify(result), 400

    if target not in TARGET_LANGUAGES:
        result["errors"] = [f"Unsupported target language: {target}"]
        return jsonify(result), 400

    try:
        proc = subprocess.Popen(
            [COMPILER_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=PROJECT_DIR,
        )

        input_data = code + "\n###END###\n" + target + "\n"
        stdout, stderr = proc.communicate(input_data, timeout=30)

        if stdout.strip():
            try:
                parsed = json.loads(stdout.strip())
                result["output"] = parsed.get("output", "")
                result["ast"] = parsed.get("ast", "")
                result["ir"] = parsed.get("ir", "")
                result["optimized_ir"] = parsed.get("optimized_ir", "")
                result["errors"] = parsed.get("errors", [])
            except json.JSONDecodeError:
                result["output"] = stdout.strip()
        elif stderr.strip():
            result["errors"] = [stderr.strip()]
        else:
            result["errors"] = ["Compiler produced no output."]

        if proc.returncode != 0 and not result["errors"]:
            result["errors"] = [f"Compiler exited with code {proc.returncode}."]

    except subprocess.TimeoutExpired:
        result["errors"] = ["Compilation timed out (30s limit)."]
    except FileNotFoundError:
        result["errors"] = [f"Compiler executable not found."]
    except Exception as e:
        result["errors"] = [f"Unexpected error: {str(e)}"]

    status_code = 200 if not result["errors"] else 422
    return jsonify(result), status_code


if __name__ == "__main__":
    app.run(debug=True, port=5000)