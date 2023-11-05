from flask import Flask

app = Flask(__name__)

@app.route("/interval")
def hello_world():
    return "0.1"

def start_server():
    app.run(host="0.0.0.0", port=8080)

if __name__ == "__main__":
    start_server()
