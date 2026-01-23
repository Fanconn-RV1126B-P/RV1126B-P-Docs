# RV1126B-P Documentation

Comprehensive documentation for Rockchip RV1126B-P development board.

## ✨ Highlights

- 🎥 **Complete IPC Workflow** - Build, flash, and deploy RKIPC firmware with RTSP streaming
- 🤖 **AI/CV Features** - Motion detection, object detection, and NPU-accelerated smart encoding
- 🚀 **Hardware Acceleration** - FFmpeg with RK MPP (H.264/H.265) and RGA (2D graphics)
- 🔧 **Production Ready** - Docker-based build environment, comprehensive testing guides
- 📖 **In-Depth Analysis** - Technical documentation of video pipeline and ISP integration

## 📚 Documentation Structure

### Hardware Documentation (From wiki.fanconn.com)
- **00.规格书** - Specifications and datasheets
- **01.硬件资料** - Hardware design files and schematics
- **02.软件资料** - Software resources
- **03.工厂测试** - Factory testing procedures
- **05.教程** - Tutorials and guides
- **RV1126 vs RV1126B** - Differences between RV1126 and RV1126B

### Software Development Guides
- **[Building Software with SDK](./guides/building-software-with-sdk.md)** - Complete guide for SDK-based development
- **[Cross-Compilation Guide](./guides/cross-compilation-guide.md)** - Using SDK toolchain for custom builds
- **[Buildroot Integration](./guides/buildroot-integration.md)** - Integrating custom packages into buildroot

#### IPC Development & Deployment
- **[Build and Flash Guide](./guides/build-and-flash.md)** - Complete build, flash, and testing workflow ⭐
- **[RKIPC Management Guide](./guides/rkipc/rkipc-management.md)** - RKIPC operations, configuration, and AI features ⭐
- **[RKIPC Video Pipeline Analysis](./guides/RKIPC_VIDEO_PIPELINE_ANALYSIS.md)** - Technical deep-dive into video pipeline

#### FFmpeg & Hardware Acceleration
- **[FFmpeg Cross-Compilation Guide](./guides/ffmpeg-rkmpp/ffmpeg-cross-compilation.md)** - Complete cross-compilation walkthrough ⭐
- **[FFmpeg Hardware Acceleration](./guides/ffmpeg-rkmpp/ffmpeg-hardware-acceleration.md)** - Usage examples and performance
- **[MPP Version and Build Status](./guides/ffmpeg-rkmpp/mpp-version-and-status.md)** - MPP v1.0.11 codec details
- **[RGA Version and Build Status](./guides/ffmpeg-rkmpp/rga-version-and-status.md)** - RGA v2.1.0 graphics acceleration

## 🚀 Quick Start

### For IPC Development
1. **Build Firmware**: See [Build and Flash Guide](./guides/build-and-flash.md) ⭐
2. **Deploy & Test**: Flash firmware and verify RTSP streaming
3. **Manage RKIPC**: See [RKIPC Management Guide](./guides/rkipc/rkipc-management.md) ⭐
4. **AI Features**: Configure motion detection, object detection, and smart encoding

### For General Development
1. **SDK Setup**: See [Building Software with SDK](./guides/building-software-with-sdk.md)
2. **Cross-Compile FFmpeg**: See [FFmpeg Cross-Compilation Guide](./guides/ffmpeg-rkmpp/ffmpeg-cross-compilation.md) ⭐
3. **Hardware Acceleration**: See [FFmpeg Hardware Acceleration](./guides/ffmpeg-rkmpp/ffmpeg-hardware-acceleration.md)
4. **MPP & RGA Details**: See [MPP Status](./guides/ffmpeg-rkmpp/mpp-version-and-status.md) and [RGA Status](./guides/ffmpeg-rkmpp/rga-version-and-status.md)

## 📖 Related Resources

- [SDK Repository](https://github.com/Fanconn-RV1126B-P/RV1126B-P-SDK)
- [FFmpeg-Rockchip Fork](https://github.com/Fanconn-RV1126B-P/ffmpeg-rockchip)
- [Docker Build Environment](https://github.com/Fanconn-RV1126B-P/RV1126B-P-SDK-Docker)

## 🤝 Contributing

Contributions to improve documentation are welcome. Please submit pull requests or open issues.

## 📄 License

See [LICENSE](LICENSE) file for details.
