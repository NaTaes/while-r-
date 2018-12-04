module.exports = (sequelize, DataTypes) => (
  sequelize.define('data', {
    nick: {
      type: DataTypes.STRING(40),
      allowNull: false,
      defaultValue: 'whiler',
      //unique: true, // 중복 불가능
    },
    emg_sensor_up: {
      type: DataTypes.STRING(10),
      allowNull: false,
      defaultValue: '0',
    },
    emg_sensor_down: {
      type: DataTypes.STRING(10),
      allowNull: false,
      defaultValue: '0',
    },
    gyro_sensor: {
      type: DataTypes.STRING(30),
      allowNull: false,
      defaultValue: '0',
    },
    created_at: {
      type: DataTypes.DATE(6),
      allowNull: false,
      defaultValue: new Date().getTime() + (540 * 60 * 1000)
    }
  }, {
      timestamps: false, // 수정일, 생성일 
      paranoid: false, // 삭제일(복구용)
      // zero 25 2018-07-20 2018-08-20
      // nero 32 2018-07-21
    })
);
