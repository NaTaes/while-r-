module.exports = (sequelize, DataTypes) => (
    sequelize.define('data', {
      email: {
        type: DataTypes.STRING(40),
        allowNull: false, 
        unique: true, // 중복 불가능
      },
      emg_sensor: {
        type: DataTypes.STRING(10),
        allowNull: false,
        defaultValue: 'local',
      },
      flex_sensor: {
        type: DataTypes.STRING(30),
        allowNull: false,
      },
    }, {
      timestamps: true, // 수정일, 생성일 
      paranoid: false, // 삭제일(복구용)
      // zero 25 2018-07-20 2018-08-20
      // nero 32 2018-07-21
    })
  );