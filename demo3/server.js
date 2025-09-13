const express = require('express');
const path = require('path');
const app = express();
const PORT = 3000;

// 中间件
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// 正常接口 - 返回200
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', message: 'Server is running' });
});

// 模拟500错误的接口
app.get('/api/error', (req, res) => {
  console.log('收到错误接口请求，返回500状态码');
  res.status(500).json({ 
    error: 'Internal Server Error',
    message: '这是一个模拟的服务器错误',
    timestamp: new Date().toISOString(),
    code: 'ERR_500_SIMULATED'
  });
});

// 模拟网络超时的接口
app.get('/api/timeout', (req, res) => {
  console.log('收到超时接口请求，延迟10秒后返回');
  setTimeout(() => {
    res.json({ message: '请求完成，但延迟了10秒' });
  }, 10000);
});

// 模拟404错误的接口
app.get('/api/notfound', (req, res) => {
  console.log('收到404接口请求');
  res.status(404).json({ 
    error: 'Not Found',
    message: '请求的资源不存在',
    code: 'ERR_404'
  });
});

// 模拟数据库连接错误的接口
app.get('/api/db-error', (req, res) => {
  console.log('收到数据库错误接口请求，返回503状态码');
  res.status(503).json({ 
    error: 'Service Unavailable',
    message: '数据库连接失败',
    code: 'ERR_DB_CONNECTION_FAILED',
    details: '无法连接到MySQL数据库服务器'
  });
});

// 根路径返回HTML页面
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// 启动服务器
app.listen(PORT, () => {
  console.log(`🚀 服务器启动成功！`);
  console.log(`📍 访问地址: http://localhost:${PORT}`);
  console.log(`📁 静态文件目录: ${path.join(__dirname, 'public')}`);
  console.log(`🔌 错误接口: http://localhost:${PORT}/api/error`);
  console.log(`⏰ 超时接口: http://localhost:${PORT}/api/timeout`);
  console.log(`❌ 404接口: http://localhost:${PORT}/api/notfound`);
  console.log(`🗄️ 数据库错误接口: http://localhost:${PORT}/api/db-error`);
  console.log(`\n按 Ctrl+C 停止服务器`);
});

// 优雅关闭
process.on('SIGINT', () => {
  console.log('\n🛑 正在关闭服务器...');
  process.exit(0);
});
