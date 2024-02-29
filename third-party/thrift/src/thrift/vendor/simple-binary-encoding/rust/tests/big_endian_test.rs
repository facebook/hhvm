use baseline_bigendian::*;
use car_codec::encoder::*;

#[test]
fn big_endian_baseline_example() -> SbeResult<()> {
    // The byte array is from the java example made by running
    // with -Dsbe.encoding.filename and then decoded using od -tu1
    let big_endian_bytes: Vec<u8> = vec![
        0, 49, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 210, 7, 221, 1, 65, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 97, 98, 99, 100, 101, 102, 6, 7, 208, 4, 49, 50, 51,
        35, 1, 78, 200, 0, 6, 0, 3, 0, 30, 66, 15, 153, 154, 0, 0, 0, 11, 85, 114, 98, 97, 110, 32,
        67, 121, 99, 108, 101, 0, 55, 66, 68, 0, 0, 0, 0, 0, 14, 67, 111, 109, 98, 105, 110, 101,
        100, 32, 67, 121, 99, 108, 101, 0, 75, 66, 32, 0, 0, 0, 0, 0, 13, 72, 105, 103, 104, 119,
        97, 121, 32, 67, 121, 99, 108, 101, 0, 1, 0, 2, 95, 0, 6, 0, 3, 0, 30, 64, 128, 0, 0, 0,
        60, 64, 240, 0, 0, 0, 100, 65, 67, 51, 51, 99, 0, 6, 0, 3, 0, 30, 64, 115, 51, 51, 0, 60,
        64, 227, 51, 51, 0, 100, 65, 60, 204, 205, 0, 0, 0, 5, 72, 111, 110, 100, 97, 0, 0, 0, 9,
        67, 105, 118, 105, 99, 32, 86, 84, 105, 0, 0, 0, 6, 97, 98, 99, 100, 101, 102,
    ];

    decode_car_and_assert_expected_content(&big_endian_bytes)?;
    let (limit, mut bytes_encoded_from_rust) = encode_car_from_scratch()?;
    decode_car_and_assert_expected_content(bytes_encoded_from_rust.as_slice())?;
    bytes_encoded_from_rust.truncate(limit);
    assert_eq!(big_endian_bytes, bytes_encoded_from_rust);

    Ok(())
}

fn decode_car_and_assert_expected_content(buffer: &[u8]) -> SbeResult<()> {
    let mut car = CarDecoder::default();

    let buf = ReadBuf::new(buffer);
    let header = MessageHeaderDecoder::default().wrap(buf, 0);
    assert_eq!(SBE_TEMPLATE_ID, header.template_id());
    car = car.header(header);

    // Car...
    assert_eq!(1234, car.serial_number());
    assert_eq!(2013, car.model_year());
    assert_eq!(BooleanType::T, car.available());
    assert_eq!(Model::A, car.code());

    assert_eq!([0, 1, 2, 3, 4], car.some_numbers());
    assert_eq!(*b"abcdef", car.vehicle_code());

    let extras = car.extras();
    assert_eq!(6, extras.0);
    assert!(extras.get_cruise_control());
    assert!(extras.get_sports_pack());
    assert!(!extras.get_sun_roof());

    assert_eq!(Model::C, car.discounted_model());

    let engine = car.engine_decoder();
    assert_eq!(2000, engine.capacity());
    assert_eq!(4, engine.num_cylinders());
    assert_eq!(9000, engine.max_rpm());
    assert_eq!("123", String::from_utf8_lossy(&engine.manufacturer_code()));

    assert_eq!(b"Petrol", engine.fuel());
    assert_eq!(35, engine.efficiency());
    assert_eq!(BooleanType::T, engine.booster_enabled());

    let mut booster = engine.booster_decoder();
    assert_eq!(BoostType::NITROUS, booster.boost_type());
    assert_eq!(200, booster.horse_power());

    car = booster.parent()?.parent()?;
    let mut fuel_figures = car.fuel_figures_decoder();
    assert_eq!(3, fuel_figures.count());

    assert_eq!(Some(0), fuel_figures.advance()?);
    assert_eq!(30, fuel_figures.speed());
    assert_eq!(35.9, fuel_figures.mpg());
    let coord = fuel_figures.usage_description_decoder();
    assert_eq!(
        "Urban Cycle",
        String::from_utf8_lossy(fuel_figures.usage_description_slice(coord))
    );

    assert_eq!(Some(1), fuel_figures.advance()?);
    assert_eq!(55, fuel_figures.speed());
    assert_eq!(49.0, fuel_figures.mpg());
    let coord = fuel_figures.usage_description_decoder();
    assert_eq!(
        "Combined Cycle",
        String::from_utf8_lossy(fuel_figures.usage_description_slice(coord))
    );

    assert_eq!(Some(2), fuel_figures.advance()?);
    assert_eq!(75, fuel_figures.speed());
    assert_eq!(40.0, fuel_figures.mpg());
    let coord = fuel_figures.usage_description_decoder();
    assert_eq!(
        "Highway Cycle",
        String::from_utf8_lossy(fuel_figures.usage_description_slice(coord))
    );
    assert_eq!(Ok(None), fuel_figures.advance());

    car = fuel_figures.parent()?;
    let mut performance_figures = car.performance_figures_decoder();
    assert_eq!(2, performance_figures.count());

    // 95 octane
    assert_eq!(Some(0), performance_figures.advance()?);
    assert_eq!(95, performance_figures.octane_rating());
    let mut acceleration = performance_figures.acceleration_decoder();
    assert_eq!(3, acceleration.count());
    assert_eq!(Some(0), acceleration.advance()?);
    assert_eq!(30, acceleration.mph());
    assert_eq!(4.0, acceleration.seconds());
    assert_eq!(Some(1), acceleration.advance()?);
    assert_eq!(60, acceleration.mph());
    assert_eq!(7.5, acceleration.seconds());
    assert_eq!(Some(2), acceleration.advance()?);
    assert_eq!(100, acceleration.mph());
    assert_eq!(12.2, acceleration.seconds());
    assert_eq!(Ok(None), acceleration.advance());

    // 99 octane
    performance_figures = acceleration.parent()?;
    assert_eq!(Some(1), performance_figures.advance()?);
    assert_eq!(99, performance_figures.octane_rating());
    acceleration = performance_figures.acceleration_decoder();
    assert_eq!(3, acceleration.count());
    assert_eq!(Some(0), acceleration.advance()?);
    assert_eq!(30, acceleration.mph());
    assert_eq!(3.8, acceleration.seconds());
    assert_eq!(Some(1), acceleration.advance()?);
    assert_eq!(60, acceleration.mph());
    assert_eq!(7.1, acceleration.seconds());
    assert_eq!(Some(2), acceleration.advance()?);
    assert_eq!(100, acceleration.mph());
    assert_eq!(11.8, acceleration.seconds());
    assert_eq!(Ok(None), acceleration.advance());

    performance_figures = acceleration.parent()?;
    car = performance_figures.parent()?;

    let coord = car.manufacturer_decoder();
    assert_eq!(
        "Honda",
        String::from_utf8_lossy(car.manufacturer_slice(coord))
    );

    let coord = car.model_decoder();
    assert_eq!("Civic VTi", String::from_utf8_lossy(car.model_slice(coord)));

    let coord = car.activation_code_decoder();
    assert_eq!(b"abcdef", car.activation_code_slice(coord));

    Ok(())
}

fn encode_car_from_scratch() -> SbeResult<(usize, Vec<u8>)> {
    let mut buffer = vec![0u8; 256];
    let mut car = CarEncoder::default();
    let mut fuel_figures = FuelFiguresEncoder::default();
    let mut performance_figures = PerformanceFiguresEncoder::default();
    let mut acceleration = AccelerationEncoder::default();
    let mut extras = OptionalExtras::default();

    car = car.wrap(
        WriteBuf::new(buffer.as_mut_slice()),
        message_header_codec::ENCODED_LENGTH,
    );
    car = car.header(0).parent()?;

    car.serial_number(1234);
    car.model_year(2013);
    car.available(BooleanType::T);
    car.code(Model::A);
    car.some_numbers([0, 1, 2, 3, 4]);
    car.vehicle_code(*b"abcdef");

    extras.set_cruise_control(true);
    extras.set_sports_pack(true);
    extras.set_sun_roof(false);
    car.extras(extras);

    let mut engine = car.engine_encoder();
    engine.capacity(2000);
    engine.num_cylinders(4);
    engine.manufacturer_code(*b"123");
    engine.efficiency(35);
    engine.booster_enabled(BooleanType::T);
    let mut booster = engine.booster_encoder();
    booster.boost_type(BoostType::NITROUS);
    booster.horse_power(200);

    engine = booster.parent()?;
    car = engine.parent()?;
    fuel_figures = car.fuel_figures_encoder(3, fuel_figures);
    assert_eq!(Some(0), fuel_figures.advance()?);
    fuel_figures.speed(30);
    fuel_figures.mpg(35.9);
    fuel_figures.usage_description("Urban Cycle");

    assert_eq!(Some(1), fuel_figures.advance()?);
    fuel_figures.speed(55);
    fuel_figures.mpg(49.0);
    fuel_figures.usage_description("Combined Cycle");

    assert_eq!(Some(2), fuel_figures.advance()?);
    fuel_figures.speed(75);
    fuel_figures.mpg(40.0);
    fuel_figures.usage_description("Highway Cycle");

    car = fuel_figures.parent()?;
    performance_figures = car.performance_figures_encoder(2, performance_figures);
    assert_eq!(Some(0), performance_figures.advance()?);
    performance_figures.octane_rating(95);

    acceleration = performance_figures.acceleration_encoder(3, acceleration);
    assert_eq!(Some(0), acceleration.advance()?);
    acceleration.mph(30);
    acceleration.seconds(4.0);

    assert_eq!(Some(1), acceleration.advance()?);
    acceleration.mph(60);
    acceleration.seconds(7.5);

    assert_eq!(Some(2), acceleration.advance()?);
    acceleration.mph(100);
    acceleration.seconds(12.2);

    performance_figures = acceleration.parent()?;
    assert_eq!(Some(1), performance_figures.advance()?);
    performance_figures.octane_rating(99);

    acceleration = performance_figures.acceleration_encoder(3, acceleration);
    assert_eq!(Some(0), acceleration.advance()?);
    acceleration.mph(30);
    acceleration.seconds(3.8);

    assert_eq!(Some(1), acceleration.advance()?);
    acceleration.mph(60);
    acceleration.seconds(7.1);

    assert_eq!(Some(2), acceleration.advance()?);
    acceleration.mph(100);
    acceleration.seconds(11.8);

    performance_figures = acceleration.parent()?;
    car = performance_figures.parent()?;

    car.manufacturer("Honda");
    car.model("Civic VTi");
    car.activation_code("abcdef");

    let limit = car.get_limit();
    Ok((limit, buffer))
}
