# Midterm Project: Implement ls(1)

## 1. Thông tin sinh viên

- **Họ tên:** Huỳnh Thị Thanh Thảo
- **Mã sinh viên:** 24IT251
- **Lớp:** SE2
- **Học phần:** Lập trình hệ thống (4)
- **Repository:** [HuynhThiThanhThao_24IT251_midterm](https://github.com/thawo278/HuynhThiThanhThao_24IT251_midterm)

## 2. Giới thiệu

Dự án xây dựng chương trình `myls` bằng ngôn ngữ C, mô phỏng một tập con chức năng của lệnh UNIX `ls(1)` theo tài liệu được cung cấp.

Chương trình sử dụng các hàm thư viện và lời gọi hệ thống để đọc thư mục, lấy thông tin tệp, sắp xếp và hiển thị kết quả. Chương trình không gọi lệnh `ls` có sẵn để thay thế việc triển khai.

## 3. Môi trường thực hiện

- Hệ điều hành: NetBSD 11.0, amd64.
- Máy ảo: Oracle VirtualBox.
- Trình biên dịch: `cc`.
- Công cụ biên dịch: `make`.
- Quản lý mã nguồn: Git và GitHub.
- Ngôn ngữ lập trình: C.

## 4. Cấu trúc mã nguồn

| File / thư mục | Chức năng |
|---|---|
| `include/myls.h` | Khai báo cấu trúc dữ liệu và các hàm dùng chung. |
| `src/main.c` | Điểm bắt đầu chương trình và xử lý đầu vào. |
| `src/options.c` | Phân tích các tùy chọn dòng lệnh. |
| `src/list.c` | Đọc thư mục, xử lý đường dẫn và duyệt đệ quy. |
| `src/sort.c` | Sắp xếp theo tên, kích thước và thời gian. |
| `src/display.c` | Hiển thị và định dạng kết quả. |
| `Makefile` | Biên dịch và dọn các tệp sinh ra. |
| `.gitignore` | Loại trừ các tệp không cần thiết khỏi Git. |

## 5. Các chức năng đã triển khai

Chương trình hỗ trợ các chức năng liệt kê tệp và thư mục, xử lý nhiều đường dẫn, hiển thị thông tin chi tiết, sắp xếp kết quả và thông báo lỗi khi không thể truy cập đường dẫn.

| Tùy chọn | Chức năng |
|---|---|
| `-a` | Hiển thị tất cả mục, bao gồm `.` và `..`. |
| `-A` | Hiển thị mục ẩn, bỏ qua `.` và `..`. |
| `-l` | Hiển thị thông tin chi tiết. |
| `-n` | Hiển thị UID và GID dưới dạng số. |
| `-i` | Hiển thị số inode. |
| `-s` | Hiển thị số block được cấp phát. |
| `-h` | Hiển thị dung lượng theo đơn vị dễ đọc. |
| `-k` | Sử dụng đơn vị 1024 byte cho số block. |
| `-F` | Thêm ký hiệu phân loại tệp. |
| `-R` | Liệt kê đệ quy các thư mục con. |
| `-d` | Hiển thị chính thư mục thay vì nội dung bên trong. |
| `-r` | Đảo ngược thứ tự sắp xếp. |
| `-S` | Sắp xếp theo kích thước giảm dần. |
| `-t` | Sắp xếp theo thời gian, mới nhất trước. |
| `-u` | Sử dụng thời gian truy cập. |
| `-c` | Sử dụng thời gian thay đổi trạng thái. |
| `-f` | Không sắp xếp. |
| `-q` | Thay ký tự không in được bằng `?`. |
| `-w` | Hiển thị tên tệp nguyên dạng. |

Chương trình cũng hỗ trợ biến môi trường `BLOCKSIZE` để lựa chọn đơn vị hiển thị block và `TZ` để điều chỉnh múi giờ hiển thị thời gian.

## 6. Cách biên dịch

Mở Terminal tại thư mục dự án và chạy:

```sh
make
```

Sau khi biên dịch thành công, chương trình được tạo với tên `myls`.

Để xóa các tệp sinh ra trong quá trình biên dịch, chạy:

```sh
make clean
```

## 7. Cách sử dụng

Liệt kê thư mục hiện tại:

```sh
./myls
```

Hiển thị thông tin chi tiết:

```sh
./myls -l
```

Hiển thị các mục ẩn:

```sh
./myls -a /etc
```

Hiển thị thông tin chi tiết với dung lượng dễ đọc:

```sh
./myls -lh /etc
```

Liệt kê thư mục theo chế độ đệ quy:

```sh
./myls -R test
```

Hiển thị UID và GID dạng số:

```sh
./myls -n test
```

Sắp xếp theo kích thước:

```sh
./myls -S test/a.txt test/b.txt
```

Đảo ngược thứ tự sắp xếp:

```sh
./myls -Sr test/a.txt test/b.txt
```

## 8. Kiểm thử

Các trường hợp kiểm thử cơ bản:

| Lệnh | Kết quả mong đợi |
|---|---|
| `./myls` | Liệt kê các mục trong thư mục hiện tại. |
| `./myls -a test` | Hiển thị cả `.` và `..`, cùng các mục ẩn. |
| `./myls -A test` | Hiển thị mục ẩn nhưng không có `.` và `..`. |
| `./myls -l test` | Hiển thị thông tin chi tiết của các mục. |
| `./myls -F test` | Thêm ký hiệu phân loại tệp. |
| `./myls -R test` | Liệt kê thư mục con theo chế độ đệ quy. |
| `./myls -d test` | Chỉ hiển thị chính thư mục `test`. |
| `./myls -S test/a.txt test/b.txt` | Tệp lớn hơn đứng trước. |
| `./myls -Sr test/a.txt test/b.txt` | Đảo ngược thứ tự sắp xếp. |
| `./myls khong_ton_tai` | Hiển thị thông báo lỗi và trả về mã trạng thái khác 0. |

Các trường hợp trên là những ca kiểm thử cơ bản; chưa bao phủ toàn bộ tổ hợp tùy chọn và mọi tình huống lỗi.

## 9. Giới hạn

- Chương trình được xây dựng và kiểm thử trong môi trường NetBSD 11.0.
- Chưa xác minh khả năng biên dịch trên tất cả hệ điều hành khác.
- Chưa kiểm thử đầy đủ mọi loại tệp đặc biệt và mọi tổ hợp tùy chọn.
- Kết quả hiển thị có thể phụ thuộc vào thông tin tệp và cấu hình môi trường.

## 10. Kết luận

Dự án giúp vận dụng kiến thức lập trình hệ thống bằng ngôn ngữ C, thao tác với hệ thống tệp trên môi trường Unix-like, tổ chức chương trình thành nhiều mô-đun, sử dụng Makefile và quản lý mã nguồn bằng GitHub.

Chương trình `myls` mô phỏng một tập con chức năng của lệnh UNIX `ls(1)`. Các chức năng được tiếp tục kiểm thử và hoàn thiện theo yêu cầu của bài tập.
