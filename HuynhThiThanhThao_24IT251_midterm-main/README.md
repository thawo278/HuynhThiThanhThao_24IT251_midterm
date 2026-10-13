# Midterm Project: Implement ls(1)

## 1. Thông tin sinh viên

- Họ tên: Phan Tuấn Anh
- Mã sinh viên: 24IT008
- Repository: https://github.com/phananh51342/PhanTuanAnh_24IT008_midterm

## 2. Giới thiệu

Dự án xây dựng chương trình `myls` bằng ngôn ngữ C, mô phỏng
phiên bản rút gọn của lệnh UNIX `ls` theo tài liệu được cung cấp.

Chương trình sử dụng các lời gọi hệ thống và hàm thư viện để
đọc thư mục, lấy thông tin file, sắp xếp và hiển thị kết quả.
Chương trình không gọi lệnh `ls` có sẵn để thực hiện chức năng.

## 3. Môi trường

- Hệ điều hành: NetBSD 11.0, amd64.
- Máy ảo: Oracle VirtualBox.
- Trình biên dịch: cc.
- Công cụ biên dịch: make.
- Quản lý phiên bản: Git và GitHub.

## 4. Cấu trúc mã nguồn

| File | Vai trò |
|---|---|
| include/myls.h | Khai báo cấu trúc dữ liệu và các hàm dùng chung |
| src/main.c | Điểm bắt đầu, xử lý đường dẫn mặc định |
| src/options.c | Phân tích tùy chọn dòng lệnh |
| src/list.c | Đọc thư mục, xử lý đường dẫn và duyệt đệ quy |
| src/sort.c | Sắp xếp theo tên, kích thước và thời gian |
| src/display.c | Hiển thị thông tin và định dạng kết quả |
| Makefile | Biên dịch và dọn các file sinh ra |
| .gitignore | Loại file biên dịch và dữ liệu thử khỏi Git |

## 5. Chức năng đã triển khai

- Liệt kê thư mục hiện tại khi không truyền đường dẫn.
- Xử lý nhiều đường dẫn; nhóm file được hiển thị trước nhóm thư mục.
- Mặc định hiển thị mỗi mục trên một dòng.
- Báo lỗi khi không truy cập được đường dẫn.
- Trả về 0 khi thành công và giá trị khác 0 khi có lỗi.

| Tùy chọn | Chức năng |
|---|---|
| -a | Hiển thị mọi mục, gồm cả . và .. |
| -A | Hiển thị mục ẩn nhưng bỏ . và .. |
| -l | Hiển thị thông tin chi tiết |
| -n | Hiển thị chi tiết với UID/GID dạng số |
| -i | Hiển thị inode |
| -s | Hiển thị số block được cấp phát |
| -h | Hiển thị dung lượng dễ đọc |
| -k | Dùng đơn vị 1024 byte cho số block |
| -F | Thêm ký hiệu phân loại file |
| -R | Liệt kê đệ quy các thư mục con |
| -d | Hiển thị chính thư mục thay vì nội dung |
| -r | Đảo thứ tự sắp xếp |
| -S | Sắp xếp theo kích thước giảm dần |
| -t | Sắp xếp theo thời gian mới nhất trước |
| -u | Sử dụng thời gian truy cập |
| -c | Sử dụng thời gian thay đổi trạng thái |
| -f | Không sắp xếp |
| -q | Thay ký tự không in được bằng dấu ? |
| -w | In tên file nguyên dạng |

Chương trình sử dụng BLOCKSIZE để chọn đơn vị hiển thị block
khi không có -h hoặc -k. Thời gian được định dạng theo múi giờ
của môi trường, có thể điều chỉnh bằng TZ.

Khi chạy bằng root, chương trình tự bật hành vi của -A.

## 6. Cách biên dịch

Trong thư mục dự án:

```sh
make
```

Chương trình tạo ra có tên `myls`.

Dọn file biên dịch:

```sh
make clean
```

## 7. Cách sử dụng

```sh
./myls
./myls -l
./myls -a /etc
./myls -lh /etc
./myls -R test
./myls -n test
./myls -S test/a.txt test/b.txt
```

Có thể kết hợp nhiều tùy chọn:

```sh
./myls -lai test
./myls -Sr test/a.txt test/b.txt
```

## 8. Kiểm thử

Dữ liệu thử gồm:
- Hai file thường có kích thước khác nhau.
- Một file ẩn.
- Một thư mục con chứa file.
- Một symbolic link.
- Một FIFO.

Các trường hợp kiểm thử cơ bản:

| Lệnh | Kết quả mong đợi |
|---|---|
| ./myls -a test | Có ., .. và .hidden |
| ./myls -A test | Có .hidden, không có . và .. |
| ./myls -l test | Có thông tin chi tiết và đích symbolic link |
| ./myls -F test | Có hậu tố phân loại |
| ./myls -R test | Hiển thị thư mục con và file bên trong |
| ./myls -d test | Chỉ hiển thị chính thư mục test |
| ./myls -S test/a.txt test/b.txt | File lớn hơn đứng trước |
| ./myls -Sr test/a.txt test/b.txt | File nhỏ hơn đứng trước |
| ./myls khong_ton_tai | Báo lỗi và trả mã khác 0 |

Các kiểm thử trên chưa bao phủ mọi tổ hợp tùy chọn,
mọi loại file đặc biệt và mọi tình huống lỗi hệ thống.

## 9. Giới hạn

- Môi trường biên dịch và chạy thử là NetBSD 11.0.
- Chưa xác minh khả năng biên dịch trên các hệ điều hành khác.
- Các loại file đặc biệt phụ thuộc khả năng hỗ trợ của hệ thống.
- Chưa kiểm thử toàn bộ các trường hợp biên và tổ hợp tùy chọn.
