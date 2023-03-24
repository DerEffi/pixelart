import moment from "moment";

export interface IStatistics {
    internal: number;
    external: number;
    time: moment.Moment;
}