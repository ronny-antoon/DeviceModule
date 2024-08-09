docker run --rm -v C:\Users\RonnyPC\Documents\chatgptcoder\DeviceModule:/project -w /project espressif/esp-matter:release-v1.3_idf_v5.2.1 

esp-matter-mfg-tool -cn "Test" --pai -k Test_Cert/Chip-Test-PAI-FFF2-8001-Key.pem -c Test_Cert/Chip-Test-PAI-FFF2-8001-Cert.pem -cd Test_Cert/Chip-Test-CD-FFF2-8001.der -v 0xFFF2 -p 0x8001 --vendor-name MyCompany --product-name Light12 --serial-num esp3333 --hw-ver 1 --hw-ver-str v1.0.0 --passcode 20202020 --discriminator 3840

esp-matter-mfg-tool -cd Test_Cert/Chip-Test-CD-FFF2-8001.der -v 0xFFF2 -p 0x8001 --vendor-name MyCompany --product-name Light12 --serial-num esp3333 --hw-ver 1 --hw-ver-str v1.0.0 --passcode 20202020 --discriminator 3840

python -m esptool erase_flash

python -m esptool write_flash 0x10000 out\fff2_8001\638ae0aa-cde8-489b-aec8-e0bcf9f65e9e\638ae0aa-cde8-489b-aec8-e0bcf9f65e9e-partition.bin

idf.py build flash monitor
