/// Returns the requested number of quantiles (i.e., the least value such that index/count values
/// are smaller).
pub fn calculate_quantiles(user_times: &[f64], nbr_quantiles: usize) -> Vec<f64> {
    // Returns the largest time in each bucket
    let mut ts = user_times.to_vec();
    ts.sort_by(|a, b| a.partial_cmp(b).unwrap());
    let step = (ts.len() - 1) as f64 / nbr_quantiles as f64;

    (0..nbr_quantiles)
        .map(|bucket| ts[((bucket + 1) as f64 * step) as usize])
        .collect()
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_quantiles_equal_step2() {
        let user_times = [1., 3., 5., 7., 9., 11.];
        let nbr_quantiles = 3;
        let quantiles = calculate_quantiles(&user_times, nbr_quantiles);

        assert_eq!(quantiles, [3., 7., 11.]);
    }

    #[test]
    fn test_quantiles_equal_step4() {
        let user_times = [0., 0.5, 1., 1., 7., 8., 8., 9., 64., 64., 64., 81.];
        let nbr_quantiles = 3;
        let quantiles = calculate_quantiles(&user_times, nbr_quantiles);

        assert_eq!(quantiles, [1., 9., 81.]);
    }

    #[test]
    fn test_quantiles_equal_uniform_4() {
        let user_times = [1., 2., 3., 4.];
        let nbr_quantiles = 4;
        let quantiles = calculate_quantiles(&user_times, nbr_quantiles);

        assert_eq!(quantiles, [1., 2., 3., 4.]);
    }

    #[test]
    fn test_quantiles_equal_uniform_3() {
        let user_times = [1., 2., 3., 4.];
        let nbr_quantiles = 3;
        let quantiles = calculate_quantiles(&user_times, nbr_quantiles);

        assert_eq!(quantiles, [2., 3., 4.]);
    }
}
