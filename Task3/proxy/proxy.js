const HOST = "https://todo.doczilla.pro"
const DEFAULT_PORT = 3000

const express = require("express");
const { createProxyMiddleware } = require("http-proxy-middleware");
const cors = require("cors");

const app = express();

app.use(cors({
    origin: "*",
    methods: ["GET"],
    allowedHeaders: ["Content-Type", "Authorization", "Accept"]
}));

app.use("/api", createProxyMiddleware({
    target: HOST,
    changeOrigin: true,
    pathRewrite: {
        "^/api": "/api",
    },
    onProxyReq: (proxyReq, req, res) => {
        proxyReq.setHeader("Accept", "application/json");
        proxyReq.setHeader("X-Custom-Header", "DZTask3");
    },
    onProxyRes: (proxyRes, req, res) => {
        proxyRes.headers["Access-Control-Allow-Origin"] = "*";
    }
}));

const PORT = process.env.PORT || DEFAULT_PORT;
app.listen(PORT, () => {
    console.log(`Server listening at http://localhost:${PORT}`);
});