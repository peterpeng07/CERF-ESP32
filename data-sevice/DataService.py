from flask import Flask, request
import gspread

sa = gspread.service_account(
    filename="/home/CerfDataService/mysite/secrets.json")
worksheet = sa.open("CERF").sheet1

app = Flask(__name__)


@app.route('/', methods=['GET'])
def home():
    return 'Hello from CERF Data Service!'


@app.route('/', methods=['POST'])
def addData():
    data = request.get_json()
    time = data['time']
    status = data['status']

    records = worksheet.get_all_values()
    row = len(records) + 1
    worksheet.update_cell(row, 1, time)
    worksheet.update_cell(row, 2, status)
    return time


# For Debug only
if __name__ == '__main__':
    app.run(debug=True, port=5432)
