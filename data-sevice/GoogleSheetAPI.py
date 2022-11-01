import gspread

sa = gspread.service_account(filename="secrets.json")
worksheet = sa.open("CERF").sheet1

# print(worksheet.row_count)
# print(worksheet.col_count)

records = worksheet.get_all_values()
print(records)
print(len(records))

row = len(records) + 1
worksheet.update_cell(row, 1, '12:00')
worksheet.update_cell(row, 2, 'on')
