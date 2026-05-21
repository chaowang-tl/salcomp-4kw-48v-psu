# Salcomp 4kW 48V PSU

基于 dsPIC33AK256MP208-I/PT 的 4kW 48V 电源模块固件，采用 LLC 谐振拓扑 + 电压模式控制 (VMC)。

## 硬件概述

| 项目 | 规格 |
|------|------|
| MCU | dsPIC33AK256MP208-I/PT (80-pin TQFP) |
| 拓扑 | LLC 谐振全桥 |
| 输出 | 48V / 83A (4kW) |
| 控制方式 | 电压模式控制 (VMC) |
| 调试器 | ICD4 |

关键外设：ADC x5、高速 PWM (PG1-PG8)、CAN、UART、Timer x2

## 软件架构

```
main.c
├── SYSTEM_Initialize()          MCC 外设初始化
├── SYSTEM_Initialize_Ext()      扩展初始化
├── TMR1 (100µs tick)            协作式调度器
│   ├── 1ms 任务
│   ├── 10ms 任务
│   └── 500ms 任务
└── 控制环路
    ├── NPNZ 数字补偿器
    ├── 软启动/斜坡
    └── LLC 状态机
```

主要模块：

- **scheduler** — 基于 100µs 定时器的协作式任务调度
- **control** — NPNZ 数字控制器、软启动、LLC 状态机
- **fault** — 故障检测与保护
- **isr** — 用户中断服务程序
- **X2Cscope** — UART 实时变量观测

## 目录结构

```
psu-4kw-48v.X/
├── main.c
├── mcc_generated_files/        MCC Melody 自动生成
│   ├── adc/
│   ├── system/
│   └── timer/
└── sources/
    ├── config/                 版本信息
    ├── control/                SMPS 控制算法
    ├── drivers/mcc_extended/   MCC 扩展驱动
    ├── fault/                  故障处理
    ├── isr/                    中断服务
    ├── scheduler/              任务调度器
    └── utils/X2Cscope/         实时调试工具
```

## 开发环境

- MPLAB X IDE
- XC-DSC Compiler v3.31
- MCC Melody v5.6.4
- DFP: dsPIC33AK-MP_DFP v1.3.185
- 调试器: MPLAB ICD4

## 构建与烧录

1. 用 MPLAB X 打开 `psu-4kw-48v.X` 项目
2. 确认目标器件为 dsPIC33AK256MP208
3. 连接 ICD4 调试器
4. Build & Program (F5)

## 固件版本

- 当前版本: v0.1.0
- 作者: Chao Wang

## 许可证

内部项目，仅限授权人员访问。
